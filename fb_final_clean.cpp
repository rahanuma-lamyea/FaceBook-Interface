#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <direct.h>    // _mkdir
#ifndef NOMINMAX
  #define NOMINMAX
#endif
#include <windows.h>
#include <conio.h>      // _getch() for hidden password input
using namespace std;

// ------------------------------------------------------------
// Utility helpers
// ------------------------------------------------------------
string toLower(string s) {
    for (char &c : s) c = (char)tolower((unsigned char)c);
    return s;
}

string readLine(const string &prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) exit(0);   // EOF safety
    return s;
}

int readInt(const string &prompt) {
    while (true) {
        string s = readLine(prompt);
        try {
            size_t pos;
            int v = stoi(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        cout << "  Invalid number, please try again.\n";
    }
}

// ------------------------------------------------------------
// Console UI: Styling & Box Helpers
// ------------------------------------------------------------
namespace ui {

const string RESET   = "\033[0m";
const string BOLD    = "\033[1m";
const string FB_BLUE = "\033[38;2;24;119;242m";   // Facebook blue
const string GRAY    = "\033[38;2;130;130;130m";
const string RED     = "\033[38;2;230;60;60m";
const string GREEN   = "\033[38;2;46;204;113m";   // Success green

const int BOX_W = 64;      // total width of a box (with the '*' borders)

void initConsole() {
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | 0x0004);  // enable ANSI colors
}

// Autoclear console cleanly
void clearScreen() {
    system("cls");
}

string rep(const string &s, int n) {
    string r;
    for (int i = 0; i < n; i++) r += s;
    return r;
}

string center(const string &s, int w) {
    if ((int)s.size() >= w) return s;
    int left = (w - (int)s.size()) / 2;
    int right = w - (int)s.size() - left;
    return string(left, ' ') + s + string(right, ' ');
}

// Helper: prints a box made with '*' borders
void box(const string &text, int w, const string &color = "", bool centered = true) {
    const string pad(2, ' ');
    int innerWidth = max(2, w - 2);
    string middle;
    if (centered) {
        middle = center(text, innerWidth);
    } else {
        middle = " " + text;
        if ((int)middle.size() < innerWidth)
            middle += string(innerWidth - (int)middle.size(), ' ');
    }
    cout << pad << GRAY << rep("*", w) << RESET << "\n";
    cout << pad << GRAY << "*" << RESET << color << BOLD << middle << RESET
         << GRAY << "*" << RESET << "\n";
    cout << pad << GRAY << rep("*", w) << RESET << "\n";
}

void drawHeader(const string &title = "FaceBook") {
    const string pad(2, ' ');
    cout << "\n";
    cout << pad << FB_BLUE << BOLD << center(title, BOX_W) << RESET << "\n\n";
}

// Masked secret input (password)
string readSecret(size_t maxLen = 50) {
    string s;
    auto onChar = [&](int ch) {
        if (ch == 8 || ch == 127) { // backspace
            if (!s.empty()) { s.pop_back(); cout << "\b \b" << flush; }
        } else if (isprint((unsigned char)ch) && s.size() < maxLen) {
            s += (char)ch;
            cout << '*' << flush;
        }
    };
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == 0 || ch == 224) { _getch(); continue; }  // ignore arrow/function keys
        onChar(ch);
    }
    cout << "\n";
    return s;
}

// ---------- Home menu: 3x3 grid matching the UI design ----------
void gridLine(int colW, int cols) {
    cout << "  +";
    for (int i = 0; i < cols; i++) cout << rep("-", colW) << "+";
    cout << "\n";
}

void gridRow(const vector<string> &cells, int colW) {
    cout << "  |";
    for (auto &c : cells) cout << BOLD << center(c, colW) << RESET << "|";
    cout << "\n";
}

void drawMenuGrid(const vector<array<string, 3>> &rows, int colW) {
    cout << "\n  " << FB_BLUE << BOLD << "FaceBook" << RESET << "\n\n";
    gridLine(colW, 3);
    for (auto &r : rows) {
        gridRow({r[0], r[1], r[2]}, colW);
        gridLine(colW, 3);
    }
    cout << "\n";
}

// ---------- Bordered section title (centered) ----------
// Draws:   +-------------+
//          |  TITLE HERE  |
//          +-------------+
void drawBoxedTitle(const string &title, const string &color = FB_BLUE) {
    int titleLen = (int)title.size();
    int innerW = titleLen + 4;  // 2 spaces padding each side
    string border = "+" + rep("-", innerW) + "+";
    int totalPad = (BOX_W - (int)border.size()) / 2;
    string padding(max(0, totalPad), ' ');
    cout << padding << border << "\n";
    cout << padding << "|  " << color << BOLD << title << RESET << "  |" << "\n";
    cout << padding << border << "\n";
}

// ---------- Post box matching the screenshot design ----------
// Draws:
//   +----------------------------------------------+
//   | #1 Alice                                     |
//   | Hello! everyone!                             |
//   |                                              |
//   | +-----------+--------------+                 |
//   | | 0 likes   | 0 comments   |                 |
//   | +-----------+--------------+                 |
//   +----------------------------------------------+
void drawPostBox(int postId, const string &authorName, const string &content,
                 int likes, int numComments,
                 const vector<pair<string, string>> &comments = {}) {
    const int W = 52;          // outer box width
    const int IW = W - 2;     // inner content width (between | and |)

    // Helper: print a padded line inside the outer box
    auto padLine = [&](const string &text, int visLen) {
        int spaces = IW - visLen;
        if (spaces < 0) spaces = 0;
        cout << "  |" << text << string(spaces, ' ') << "|\n";
    };

    string outerBorder = "+" + rep("-", IW) + "+";

    // Top border
    cout << "  " << outerBorder << "\n";

    // #id AuthorName
    string header = " #" + to_string(postId) + " " + authorName;
    cout << "  |" << BOLD << header << RESET;
    cout << string(max(0, IW - (int)header.size()), ' ') << "|\n";

    // Content — wrap long lines
    string contentLine = " " + content;
    if ((int)contentLine.size() <= IW) {
        padLine(contentLine, (int)contentLine.size());
    } else {
        // simple word-wrap
        int pos = 0;
        while (pos < (int)contentLine.size()) {
            string chunk = contentLine.substr(pos, IW);
            padLine(chunk, (int)chunk.size());
            pos += IW;
        }
    }

    // Empty line
    padLine("", 0);

    // Likes/Comments sub-box
    string likesStr = " " + to_string(likes) + " likes ";
    string commStr = " " + to_string(numComments) + " comments ";
    int subW1 = (int)likesStr.size();
    int subW2 = (int)commStr.size();
    string subBorder = "+" + rep("-", subW1) + "+" + rep("-", subW2) + "+";
    string subMiddle = "|" + likesStr + "|" + commStr + "|";

    string subPad = " ";
    padLine(subPad + subBorder, 1 + (int)subBorder.size());
    padLine(subPad + subMiddle, 1 + (int)subMiddle.size());
    padLine(subPad + subBorder, 1 + (int)subBorder.size());

    // Bottom border
    cout << "  " << outerBorder << "\n";

    // Show comments below the box (if any)
    for (auto &c : comments) {
        cout << "     " << GRAY << c.first << ": " << c.second << RESET << "\n";
    }
}

}

// ------------------------------------------------------------

// ------------------------------------------------------------
template <typename T>
class ArrayList {
    T *data = nullptr;
    int cap = 0;
    int len = 0;

    void resize(int newCap) {
        T *newData = new T[newCap];
        for (int i = 0; i < len; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
        cap = newCap;
    }

public:
    ArrayList(int initialCap = 4) : cap(initialCap), len(0) {
        data = new T[cap];
    }
    ~ArrayList() {
        delete[] data;
    }
    ArrayList(const ArrayList &other) {
        cap = other.cap;
        len = other.len;
        data = new T[cap];
        for (int i = 0; i < len; i++) data[i] = other.data[i];
    }
    ArrayList &operator=(const ArrayList &other) {
        if (this != &other) {
            delete[] data;
            cap = other.cap;
            len = other.len;
            data = new T[cap];
            for (int i = 0; i < len; i++) data[i] = other.data[i];
        }
        return *this;
    }

    void push_back(const T &val) {
        if (len == cap) resize(cap == 0 ? 4 : cap * 2);
        data[len++] = val;
    }

    int size() const { return len; }
    bool empty() const { return len == 0; }

    T &operator[](int idx) { return data[idx]; }
    const T &operator[](int idx) const { return data[idx]; }

    void clear() { len = 0; }
};

// ------------------------------------------------------------
     //Merge Sort Part For trending Post
// ------------------------------------------------------------
template <typename T, typename Compare>
void merge(ArrayList<T> &arr, int left, int mid, int right, Compare comp) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    ArrayList<T> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L.push_back(arr[left + i]);   //left array te left side store kortese
    for (int j = 0; j < n2; j++) R.push_back(arr[mid + 1 + j]); // Right array te right side store kortese

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (comp(L[i], R[j])) arr[k++] = L[i++];  // compare kore main array te jasse
        else arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];   // reamaining elements...
    while (j < n2) arr[k++] = R[j++];
}

template <typename T, typename Compare>
void mergeSort(ArrayList<T> &arr, int left, int right, Compare comp) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid, comp);             //divide kortese
        mergeSort(arr, mid + 1, right, comp);
        merge(arr, left, mid, right, comp);
    }
}

// ------------------------------------------------------------
     //Quick Sort For Mutual Friends
// ------------------------------------------------------------
template <typename T, typename Compare>
int partition(ArrayList<T> &arr, int low, int high, Compare comp) {
    T pivot = arr[high]; //Pivot always last element k nitesi.... r eta Lomuto partition scheme
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (comp(arr[j], pivot)) {   //arr[j] ki pivot er age boshbe??
            i++;
            std::swap(arr[i], arr[j]); // true hole pivot er left e boshbe
        }
    }
    std::swap(arr[i + 1], arr[high]);  // pivot k majhe dibo..
    return i + 1;  // final index pivot r
}

template <typename T, typename Compare>
void quickSort(ArrayList<T> &arr, int low, int high, Compare comp) {
    if (low < high) {
        int pi = partition(arr, low, high, comp);
        quickSort(arr, low, pi - 1, comp);
        quickSort(arr, pi + 1, high, comp);
    }
}

// ------------------------------------------------------------

// ------------------------------------------------------------
template <typename T>
class Stack {
    struct Node {
        T data;
        Node *next;
        Node(const T &d, Node *n) : data(d), next(n) {}
    };
    Node *topNode = nullptr;
    int count = 0;

public:
    Stack() = default;
    Stack(const Stack &) = delete;
    Stack &operator=(const Stack &) = delete;
    ~Stack() { while (!empty()) pop(); }

    void push(const T &v) { topNode = new Node(v, topNode); count++; }
    void pop() {
        if (empty()) return;
        Node *t = topNode;
        topNode = topNode->next;
        delete t;
        count--;
    }
    T &top() { return topNode->data; }
    bool empty() const { return topNode == nullptr; }
    int size() const { return count; }

    template <typename F> void forEach(F f) const {
        for (Node *c = topNode; c; c = c->next) f(c->data);
    }
};

// ------------------------------------------------------------

// ------------------------------------------------------------
template <typename T>
class Queue {
    struct Node {
        T data;
        Node *next;
        Node(const T &d) : data(d), next(nullptr) {}
    };
    Node *head = nullptr, *tail = nullptr;
    int count = 0;

public:
    Queue() = default;
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;
    ~Queue() { while (!empty()) dequeue(); }

    void enqueue(const T &v) {
        Node *n = new Node(v);
        if (tail) tail->next = n; else head = n;
        tail = n;
        count++;
    }
    void dequeue() {
        if (empty()) return;
        Node *t = head;
        head = head->next;
        if (!head) tail = nullptr;
        delete t;
        count--;
    }
    T &front() { return head->data; }
    bool empty() const { return head == nullptr; }
    int size() const { return count; }

    bool contains(const T &v) const {
        for (Node *c = head; c; c = c->next) if (c->data == v) return true;
        return false;
    }

    template <typename F> void forEach(F f) const {
        for (Node *c = head; c; c = c->next) f(c->data);
    }
};

// ------------------------------------------------------------

// ------------------------------------------------------------
struct User {
    int id;
    string name, username, password;
    Queue<int> friendRequests;
    Stack<string> notifications;

    User(int i, string n, string u, string p)
        : id(i), name(move(n)), username(move(u)), password(move(p)) {}
};

// ------------------------------------------------------------

// ------------------------------------------------------------
struct Post {
    int id;
    int authorId;
    string content;
    int likes = 0;
    unordered_set<int> likedBy;
    vector<pair<int, string>> comments;   // (userId, text)
    Post *next = nullptr;
};

class PostList {
    Post *head = nullptr;
    int nextId = 1;

public:
    ~PostList() {
        while (head) { Post *t = head; head = head->next; delete t; }
    }
    Post *getHead() const { return head; }

    Post *addPost(int authorId, const string &content) {
        Post *p = new Post();
        p->id = nextId++;
        p->authorId = authorId;
        p->content = content;
        p->next = head;               // insert at head -> newest first
        head = p;
        return p;
    }
    Post *findById(int id) const {
        for (Post *c = head; c; c = c->next) if (c->id == id) return c;
        return nullptr;
    }
};

// ------------------------------------------------------------

// ------------------------------------------------------------
class Graph {
    vector<vector<int>> adj;

public:
    void addUser() { adj.push_back({}); }

    bool areFriends(int a, int b) const {
        if (a < 0 || b < 0 || a >= (int)adj.size() || b >= (int)adj.size()) return false;
        const auto &v = adj[a];
        return find(v.begin(), v.end(), b) != v.end();
    }
    void addEdge(int a, int b) {
        if (areFriends(a, b)) return;
        adj[a].push_back(b);
        adj[b].push_back(a);
    }
    const vector<int> &friends(int u) const { return adj[u]; }


    void bfs(int src, vector<int> &dist, vector<int> &parent, int maxDepth = -1) const {
        int n = (int)adj.size();
        dist.assign(n, -1);
        parent.assign(n, -1);
        Queue<int> q;
        dist[src] = 0;
        q.enqueue(src);
        while (!q.empty()) {
            int u = q.front();
            q.dequeue();
            if (maxDepth != -1 && dist[u] >= maxDepth) continue;
            for (int v : adj[u]) {
                if (dist[v] == -1) {
                    dist[v] = dist[u] + 1;
                    parent[v] = u;
                    q.enqueue(v);
                }
            }
        }
    }

    vector<pair<int, int>> dfs(int src) const {
        vector<pair<int, int>> order;
        vector<bool> visited(adj.size(), false);
        Stack<pair<int, int>> st;           // (node, depth)
        st.push({src, 0});
        while (!st.empty()) {
            pair<int, int> cur = st.top();
            st.pop();
            if (visited[cur.first]) continue;
            visited[cur.first] = true;
            order.push_back(cur);
            // push in reverse so the smallest id is visited first
            const vector<int> &nb = adj[cur.first];
            for (int i = (int)nb.size() - 1; i >= 0; i--)
                if (!visited[nb[i]]) st.push({nb[i], cur.second + 1});
        }
        return order;
    }
};

// ------------------------------------------------------------

// ------------------------------------------------------------
class UserBST {
    struct Node {
        string key;
        int id;
        Node *left, *right;
    };
    Node *root = nullptr;

    Node *insert(Node *n, const string &k, int id) {
        if (!n) return new Node{k, id, nullptr, nullptr};
        if (k < n->key) n->left = insert(n->left, k, id);
        else if (k > n->key) n->right = insert(n->right, k, id); // eta bst main algorithm ta....
        return n;
    }
    void inorderPrefix(Node *n, const string &p, vector<int> &out) const {
        if (!n) return;
        inorderPrefix(n->left, p, out);
        if (n->key.compare(0, p.size(), p) == 0) out.push_back(n->id);  // eikhane prefix diye tulona kortese....and inorder traversal er madhome search kora hocche....
        inorderPrefix(n->right, p, out);
    }
    void destroy(Node *n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;  // destroy te postorder traversal hocche
    }

public:
    ~UserBST() { destroy(root); }
    void insert(const string &key, int id) { root = insert(root, key, id); }

    vector<int> searchPrefix(const string &prefix) const {
        vector<int> out;
        inorderPrefix(root, toLower(prefix), out);   // alphabetically sorted
        return out;
    }
};

// ------------------------------------------------------------

// ------------------------------------------------------------
class FacebookApp {
    vector<User *> users;
    unordered_map<string, int> loginMap;
    Graph graph;
    UserBST bst;
    PostList posts;
    int current = -1;

    int addUser(const string &name, const string &username, const string &pass) {
        int id = (int)users.size();
        users.push_back(new User(id, name, username, pass));
        graph.addUser();
        bst.insert(toLower(username), id);
        loginMap[toLower(username)] = id;
        return id;
    }

    void notify(int userId, const string &msg) {
        if (userId >= 0 && userId < (int)users.size())
            users[userId]->notifications.push(msg);
    }

    void printPost(const Post *p) const {
        vector<pair<string, string>> commentPairs;
        for (auto &c : p->comments)
            commentPairs.push_back({users[c.first]->name, c.second});
        ui::drawPostBox(p->id, users[p->authorId]->name, p->content,
                        p->likes, (int)p->comments.size(), commentPairs);
    }

    bool canSee(const Post *p) const {
        return p->authorId == current || graph.areFriends(current, p->authorId);
    }

    // ---------- Auth: Registration Flow ----------
    void registerFlow() {
        ui::clearScreen();
        ui::drawHeader("F A C E B O O K   R E G I S T E R");
        ui::box("CREATE NEW ACCOUNT", ui::BOX_W, ui::FB_BLUE, true);
        cout << "\n";

        string name;
        while (true) {
            name = readLine("  Enter Full Name : ");
            if (!name.empty()) break;
            cout << "  " << ui::RED << "Name cannot be empty.\n" << ui::RESET;
        }

        string uname;
        while (true) {
            uname = readLine("  Enter Username  : ");
            if (uname.empty()) {
                cout << "  " << ui::RED << "Username cannot be empty.\n" << ui::RESET;
                continue;
            }
            if (loginMap.count(toLower(uname))) {
                cout << "  " << ui::RED << "Username already taken! Please choose another.\n" << ui::RESET;
                continue;
            }
            break;
        }

        string pass;
        while (true) {
            cout << "  Enter Password  : ";
            pass = ui::readSecret(50);
            if (!pass.empty()) break;
            cout << "  " << ui::RED << "Password cannot be empty.\n" << ui::RESET;
        }

        addUser(name, uname, pass);
        saveData();

        cout << "\n";
        ui::box("REGISTRATION DONE!", ui::BOX_W, ui::GREEN, true);
        cout << "\n";
        cout << "  " << ui::GREEN << "Account successfully created for " << name << "!" << ui::RESET << "\n";
        cout << "  Please log in with your credentials to access FaceBook.\n\n";
        readLine("  Press Enter to proceed to Login...");
    }

    bool tryLogin(const string &uname, const string &pass) {
        auto it = loginMap.find(toLower(uname));
        if (it == loginMap.end() || users[it->second]->password != pass) return false;
        current = it->second;
        return true;
    }


    void loginFlow() {
        while (true) {
            ui::clearScreen();
            ui::drawHeader("F A C E B O O K   L O G I N");
            ui::box("USER LOGIN", ui::BOX_W, ui::FB_BLUE, true);
            cout << "\n";
            cout << "  (Type /back to return to the welcome screen)\n\n";

            string uname = readLine("  Username : ");
            if (uname == "/back" || uname == "/exit") return;
            if (uname.empty()) {
                cout << "  " << ui::RED << "Username cannot be empty." << ui::RESET << "\n";
                readLine("  Press Enter to try again...");
                continue;
            }

            auto it = loginMap.find(toLower(uname));
            if (it == loginMap.end()) {
                // User does not exist!
                cout << "\n  " << ui::RED << "[!] User '@" << uname << "' is not registered yet!" << ui::RESET << "\n";
                cout << "  You need an account to log in.\n\n";
                ui::box("DO YOU WANT TO REGISTER?", 36, ui::FB_BLUE, true);
                cout << "\n";
                cout << "  1) Yes, Register Now\n";
                cout << "  2) Try Another Username\n";
                cout << "  3) Back to Welcome Screen\n\n";
                int regChoice = readInt("  Choose option (1-3): ");
                if (regChoice == 1) {
                    registerFlow();

                    continue;
                } else if (regChoice == 3) {
                    return;
                } else {
                    continue;
                }
            }

            //
            cout << "  Password : ";
            string pass = ui::readSecret(50);

            if (tryLogin(uname, pass)) {
                ui::clearScreen();
                cout << "\n  " << ui::GREEN << ui::BOLD << "Welcome back, " << users[current]->name << "!" << ui::RESET << "\n";
                readLine("  Press Enter to open your Feed...");
                mainMenu();
                return;
            } else {
                cout << "\n  " << ui::RED << "Incorrect password. Please try again." << ui::RESET << "\n";
                readLine("  Press Enter to try again...");
            }
        }
    }

    void showNewsFeed() {
        cout << "\n";
        ui::drawBoxedTitle("NEWS FEED");
        cout << "\n";
        bool any = false;
        for (Post *p = posts.getHead(); p; p = p->next) {
            if (canSee(p)) { printPost(p); cout << "\n"; any = true; }
        }
        if (!any) {
            cout << "  Nothing to show. Add friends or create a post!\n";
            return;
        }

        cout << "\n  Do you want to react or comment any post ?\n";
        cout << "  1. yes\n";
        cout << "  2. no\n";
        int c = readInt("  enter option: ");
        if (c == 1) {
            likeOrComment();
        }
    }

    void createPost() {
        cout << "\n";
        ui::drawBoxedTitle("CREATE POST");
        cout << "\n";
        string text = readLine("  What's on your mind? ");
        if (text.empty()) { cout << "  Empty post discarded.\n"; return; }
        posts.addPost(current, text);
        for (int f : graph.friends(current))
            notify(f, users[current]->name + " shared a new post.");
        saveData();
        cout << "\n  " << ui::GREEN << "Post successfully published!" << ui::RESET << "\n";
    }

    void likeOrComment() {
        // Outer loop: pick a post (0 = back to main menu)
        while (true) {
            Post *p = nullptr;
            while (true) {
                int id = readInt("  Enter Post ID (0 to go back to menu): ");
                if (id == 0) return;
                p = posts.findById(id);
                if (p && canSee(p)) break;
                cout << "  " << ui::RED << "Post not found or inaccessible. Please try again." << ui::RESET << "\n";
            }

            // Inner loop: stay on this post, so Like and Comment can both be done
            while (true) {
                int ch = readInt("  1) Like   2) Comment   3) Another post   0) Back to menu : ");
                if (ch == 0) return;   // straight back to the main menu
                if (ch == 3) break;    // pick a different post
                if (ch == 1) {
                    if (p->likedBy.count(current)) {
                        cout << "  You have already liked this post.\n";
                    } else {
                        p->likedBy.insert(current);
                        p->likes++;
                        if (p->authorId != current)
                            notify(p->authorId, users[current]->name + " liked your post #" + to_string(p->id) + ".");
                        saveData();
                        cout << "  " << ui::GREEN << "Liked!" << ui::RESET << "\n";
                    }
                } else if (ch == 2) {
                    string c = readLine("  Comment: ");
                    if (c.empty()) {
                        cout << "  Empty comment discarded.\n";
                    } else {
                        p->comments.push_back({current, c});
                        if (p->authorId != current)
                            notify(p->authorId, users[current]->name + " commented on your post #" + to_string(p->id) + ".");
                        saveData();
                        cout << "  " << ui::GREEN << "Comment added!" << ui::RESET << "\n";
                    }
                } else {
                    cout << "  " << ui::RED << "Invalid choice. Please enter 1, 2, 3 or 0." << ui::RESET << "\n";
                    continue;
                }
                cout << "  Post #" << p->id << " now has " << p->likes << " likes, "
                     << p->comments.size() << " comments.\n";
            }
        }
    }


    void searchUsers() {
        while (true) {
            cout << "\n";
            ui::drawBoxedTitle("SEARCH USERS");
            cout << "\n";
            cout << "  1) Prefix Search by Username\n";
            cout << "  2) View All Users Sorted by Name\n";
            cout << "  3) Back to Menu\n";
            int ch = readInt("  enter option: ");

            if (ch == 3) return;

            if (ch == 2) {
                cout << "\n";
                ui::drawBoxedTitle("ALL REGISTERED USERS");
                cout << "\n";
                ArrayList<int> allUsers;
                for (int i = 0; i < (int)users.size(); i++) allUsers.push_back(i);
                quickSort(allUsers, 0, allUsers.size() - 1, [&](int a, int b) {
                    return users[a]->name < users[b]->name;
                });
                for (int i = 0; i < allUsers.size(); i++) {
                    int id = allUsers[i];
                    cout << "   " << (i + 1) << ". " << users[id]->name << " (@" << users[id]->username << ")";
                    if (id == current) cout << " [you]";
                    else if (graph.areFriends(current, id)) cout << " [friend]";
                    cout << "\n";
                }
            } else if (ch == 1) {
                string q = readLine("  Search username (prefix): ");
                vector<int> res = bst.searchPrefix(q);
                if (res.empty()) {
                    cout << "  No users found matching prefix '" << q << "'.\n";
                } else {
                    cout << "  Results:\n";
                    for (int id : res) {
                        cout << "   - " << users[id]->name << " (@" << users[id]->username << ")";
                        if (id == current) cout << " [you]";
                        else if (graph.areFriends(current, id)) cout << " [friend]";
                        cout << "\n";
                    }
                }
            } else {
                cout << "  " << ui::RED << "Invalid option. Please choose 1, 2, or 3." << ui::RESET << "\n";
            }
        }
    }

    void sendFriendRequest() {
        cout << "\n";
        ui::drawBoxedTitle("SEND FRIEND REQUEST");
        cout << "\n";
        cout << "  Tip: you can enter one or many usernames at once,\n";
        cout << "       separated by comma or space (e.g. bushra, cawsar, deya)\n\n";
        string line = readLine("  Username(s) to add: ");

        // Split the input into individual usernames
        vector<string> names;
        string cur;
        for (char ch : line + ",") {
            if (ch == ',' || ch == ' ' || ch == ';' || ch == '\t') {
                if (!cur.empty()) { names.push_back(cur); cur.clear(); }
            } else {
                cur += ch;
            }
        }
        if (names.empty()) { cout << "  No username entered.\n"; return; }

        cout << "\n";
        unordered_set<int> handled;
        int sent = 0;
        for (const string &uname : names) {
            auto it = loginMap.find(toLower(uname));
            if (it == loginMap.end()) {
                cout << "  " << ui::RED << "[x] '" << uname << "' - user not found." << ui::RESET << "\n";
                continue;
            }
            int t = it->second;
            if (handled.count(t)) continue;
            handled.insert(t);
            if (t == current) { cout << "  " << ui::RED << "[x] " << users[t]->name << " - you cannot add yourself." << ui::RESET << "\n"; continue; }
            if (graph.areFriends(current, t)) { cout << "  [-] " << users[t]->name << " - already friends.\n"; continue; }
            if (users[t]->friendRequests.contains(current)) { cout << "  [-] " << users[t]->name << " - request already sent.\n"; continue; }
            users[t]->friendRequests.enqueue(current);
            notify(t, users[current]->name + " sent you a friend request.");
            sent++;
            cout << "  " << ui::GREEN << "[+] Friend request sent to " << users[t]->name << "!" << ui::RESET << "\n";
        }
        if (sent > 0) saveData();
        cout << "\n  Total requests sent: " << sent << "\n";
    }

    void handleFriendRequests() {
        cout << "\n";
        ui::drawBoxedTitle("FRIEND REQUESTS");
        cout << "\n";
        auto &q = users[current]->friendRequests;
        if (q.empty()) { cout << "  No pending friend requests.\n"; return; }
        while (!q.empty()) {
            int s = q.front();
            cout << "\n  Request from " << users[s]->name << "\n";
            string ch = toLower(readLine("  [a]ccept / [r]eject / [s]top : "));
            if (ch == "a") {
                graph.addEdge(current, s);
                notify(s, users[current]->name + " accepted your friend request.");
                q.dequeue();
                saveData();
                cout << "  " << ui::GREEN << "You are now friends!" << ui::RESET << "\n";
            } else if (ch == "r") {
                q.dequeue();
                saveData();
                cout << "  Request rejected.\n";
            } else break;
        }
    }

    void showFriends() {
        cout << "\n";
        ui::drawBoxedTitle("MY FRIENDS");
        cout << "\n";
        const auto &f = graph.friends(current);
        cout << "  Total Friends: " << f.size() << "\n";
        cout << "\n";
        if (f.empty()) {
            cout << "  No friends yet. Add friends using Search or check Friend Requests!\n";
            return;
        }

        ArrayList<int> friendArr;
        for (int id : f) friendArr.push_back(id);

        quickSort(friendArr, 0, friendArr.size() - 1, [&](int a, int b) {
            return users[a]->name < users[b]->name;
        });

        for (int i = 0; i < friendArr.size(); i++) {
            int id = friendArr[i];
            cout << "   " << (i + 1) << ". " << users[id]->name << "\n";
        }

        cout << "\n  Do you want to see People You May Know?\n";
        cout << "  1. yes\n";
        cout << "  2. no\n";
        int c = readInt("  enter option: ");
        if (c == 1) showSuggestions();
    }


    void showSuggestions() {
        cout << "\n";
        ui::drawBoxedTitle("PEOPLE YOU MAY KNOW");
        cout << "\n";

        vector<int> dist, parent;
        graph.bfs(current, dist, parent, 2);

        ArrayList<pair<int, int>> suggArr;   // (userId, mutual friends)
        for (int u = 0; u < (int)dist.size(); u++) {
            if (dist[u] != 2) continue;      // exactly 2 steps away = friend of a friend
            int mutual = 0;
            for (int f : graph.friends(current))
                if (graph.areFriends(f, u)) mutual++;
            suggArr.push_back({u, mutual});
        }

        if (suggArr.size() == 0) {
            cout << "  No suggestions available right now.\n";
            return;
        }

        quickSort(suggArr, 0, suggArr.size() - 1, [](const pair<int, int> &a, const pair<int, int> &b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first;
        });

        for (int i = 0; i < suggArr.size(); i++) {
            auto &p = suggArr[i];
            cout << "   - " << users[p.first]->name << " (@" << users[p.first]->username
                 << ") - " << p.second << " mutual friend(s)\n";
        }
    }

    void showConnectionPath() {
        cout << "\n";
        ui::drawBoxedTitle("CONNECTION PATH");
        cout << "\n";
        string uname = readLine("  Username to check connection with: ");
        auto it = loginMap.find(toLower(uname));
        if (it == loginMap.end()) { cout << "  User not found.\n"; return; }
        int target = it->second;
        if (target == current) { cout << "  That is you!\n"; return; }

        vector<int> dist, parent;
        graph.bfs(current, dist, parent);

        cout << "\n";
        if (dist[target] == -1) {
            cout << "  No connection found between you and " << users[target]->name << ".\n";
            return;
        }

        // Walk back through parent[] and use a Stack to reverse the path
        Stack<int> path;
        for (int v = target; v != -1; v = parent[v]) path.push(v);

        cout << "  " << users[target]->name << " is " << dist[target] << " step(s) away from you.\n\n";
        cout << "  Path: ";
        bool first = true;
        while (!path.empty()) {
            if (!first) cout << " -> ";
            int id = path.top();
            path.pop();
            cout << (id == current ? string("You") : users[id]->name);
            first = false;
        }
        cout << "\n";
        if (dist[target] == 1) cout << "  (You are already friends)\n";
    }

    // DFS: explore the whole friend network going as deep as possible first
    void exploreNetwork() {
        cout << "\n";
        ui::drawBoxedTitle("EXPLORE FRIEND NETWORK");
        cout << "\n";
        vector<pair<int, int>> order = graph.dfs(current);
        if (order.size() <= 1) {
            cout << "  You have no friends yet, so there is no network to explore.\n";
            return;
        }
        cout << "  Your friend network (each level = one more step away from you):\n\n";
        for (auto &p : order) {
            if (p.first == current) { cout << "  You\n"; continue; }
            cout << "  " << string(p.second * 3, ' ') << "-> " << users[p.first]->name << "\n";
        }
        cout << "\n  People reachable in your network: " << (order.size() - 1) << "\n";
    }

    void showNotifications() {
        cout << "\n";
        ui::drawBoxedTitle("NOTIFICATIONS");
        cout << "\n";
        auto &st = users[current]->notifications;
        cout << "  Total: " << st.size() << "\n\n";
        if (st.empty()) { cout << "  No notifications.\n"; return; }
        st.forEach([](const string &m) { cout << "   * " << m << "\n"; });
        cout << "\n  Do you want to clear all notifications?\n";
        cout << "  1. yes\n";
        cout << "  2. no\n";
        int c = readInt("  enter option: ");
        if (c == 1) {
            while (!st.empty()) st.pop();
            saveData();
            cout << "  Notifications cleared.\n";
        }
    }

    void showTrending() {
        cout << "\n";
        ui::drawBoxedTitle("TRENDING POSTS");
        cout << "\n";

        ArrayList<Post *> list;
        for (Post *p = posts.getHead(); p; p = p->next) {
            if (canSee(p)) list.push_back(p);
        }

        if (list.empty()) {
            cout << "  No posts available to display.\n";
            return;
        }

        // Sort descending by likes; tie-break by number of comments
        mergeSort(list, 0, list.size() - 1, [](Post *a, Post *b) {
            if (a->likes != b->likes) return a->likes > b->likes;
            return a->comments.size() > b->comments.size();
        });

        int limit = min(list.size(), 5);
        cout << "  Showing Top " << limit << " Trending Post(s):\n\n";
        for (int i = 0; i < limit; i++) {
            printPost(list[i]);
            cout << "\n";
        }

        cout << "\n  Do you want to react or comment any post ?\n";
        cout << "  1. yes\n";
        cout << "  2. no\n";
        int c = readInt("  enter option: ");
        if (c == 1) {
            likeOrComment();
        }
    }

    // ---------- Dashboard Menu ----------
    void mainMenu() {
        const vector<array<string, 3>> menuRows = {
            {"1.NEWS FEED", "2.CREATE POST", "3.SEARCH USERS"},
            {"4.SEND FRIEND REQUEST", "5.FRIEND REQUESTS", "6.MY FRIENDS"},
            {"7.NOTIFICATIONS", "8.TRENDING POST", "9.PEOPLE YOU MAY KNOW"},
            {"10.CONNECTION PATH", "11.EXPLORE NETWORK", "12.LOG OUT"}
        };

        while (current != -1) {
            ui::clearScreen();   // Autoclear console
            ui::drawMenuGrid(menuRows, 24);
            cout << "  " << ui::FB_BLUE << ui::BOLD << "Choose Option : " << ui::RESET;
            int choice = readInt("");
            ui::clearScreen();   // Autoclear to work on the selected option

            switch (choice) {
                case 1: showNewsFeed(); break;
                case 2: createPost(); break;
                case 3: searchUsers(); break;
                case 4: sendFriendRequest(); break;
                case 5: handleFriendRequests(); break;
                case 6: showFriends(); break;
                case 7: showNotifications(); break;
                case 8: showTrending(); break;
                case 9: showSuggestions(); break;
                case 10: showConnectionPath(); break;
                case 11: exploreNetwork(); break;
                case 12:
                    current = -1;
                    saveData();
                    ui::clearScreen();
                    cout << "\n  " << ui::GREEN << "Logged out successfully." << ui::RESET << "\n";
                    readLine("  Press Enter to continue...");
                    return;
                default:
                    cout << "  " << ui::RED << "Invalid choice. Please select 1-12." << ui::RESET << "\n";
            }
            readLine("\n  Press Enter to return to the menu...");
        }
    }

    // ---------- File Storage: Save & Load ----------
    const string DATA_DIR = "data";

    string path(const string &file) const { return DATA_DIR + "/" + file; }

    static string sanitize(string s) {
        for (char &c : s)
            if (c == '|' || c == '\n' || c == '\r') c = ' ';
        return s;
    }

    void saveUsers() const {
        ofstream f(path("users.txt"));
        for (User *u : users)
            f << u->id << '|' << sanitize(u->name) << '|' << sanitize(u->username)
              << '|' << sanitize(u->password) << '\n';
    }

    void saveFriends() const {
        ofstream f(path("friends.txt"));
        for (int a = 0; a < (int)users.size(); a++)
            for (int b : graph.friends(a))
                if (a < b) f << a << ' ' << b << '\n';
    }

    void saveRequests() const {
        ofstream f(path("requests.txt"));
        for (User *u : users)
            u->friendRequests.forEach([&](int sender) { f << u->id << '|' << sender << '\n'; });
    }

    void saveNotifications() const {
        ofstream f(path("notifications.txt"));
        for (User *u : users) {
            vector<string> msgs;
            u->notifications.forEach([&](const string &m) { msgs.push_back(m); });
            for (auto it = msgs.rbegin(); it != msgs.rend(); ++it)
                f << u->id << '|' << sanitize(*it) << '\n';
        }
    }

    void savePosts() const {
        ofstream f(path("posts.txt"));
        vector<Post *> ordered;
        for (Post *p = posts.getHead(); p; p = p->next) ordered.push_back(p);
        for (auto it = ordered.rbegin(); it != ordered.rend(); ++it) {
            Post *p = *it;
            f << "POSTSTART\n" << p->id << '|' << p->authorId << '|' << p->likes << '\n';
            f << sanitize(p->content) << '\n';
            f << "LIKEDBY|";
            bool first = true;
            for (int id : p->likedBy) { if (!first) f << ','; f << id; first = false; }
            f << '\n';
            f << "COMMENTS|" << p->comments.size() << '\n';
            for (auto &c : p->comments) f << c.first << '|' << sanitize(c.second) << '\n';
            f << "POSTEND\n";
        }
    }

    static void ensureDataDir(const string &dir) {
        _mkdir(dir.c_str());
    }

    void saveData() const {
        ensureDataDir(DATA_DIR);
        saveUsers();
        saveFriends();
        saveRequests();
        saveNotifications();
        savePosts();
    }

    vector<string> splitPipe(const string &line) const {
        vector<string> parts;
        stringstream ss(line);
        string tok;
        while (getline(ss, tok, '|')) parts.push_back(tok);
        return parts;
    }

    bool loadUsers() {
        ifstream f(path("users.txt"));
        if (!f) return false;
        string line;
        bool any = false;
        while (getline(f, line)) {
            if (line.empty()) continue;
            auto p = splitPipe(line);
            if (p.size() < 4) continue;
            addUser(p[1], p[2], p[3]);
            any = true;
        }
        return any;
    }

    void loadFriends() {
        ifstream f(path("friends.txt"));
        int a, b;
        while (f >> a >> b)
            if (a >= 0 && b >= 0 && a < (int)users.size() && b < (int)users.size())
                graph.addEdge(a, b);
    }

    void loadRequests() {
        ifstream f(path("requests.txt"));
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            auto p = splitPipe(line);
            if (p.size() < 2) continue;
            int target = stoi(p[0]), sender = stoi(p[1]);
            if (target >= 0 && target < (int)users.size()) users[target]->friendRequests.enqueue(sender);
        }
    }

    void loadNotifications() {
        ifstream f(path("notifications.txt"));
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            auto p = splitPipe(line);
            if (p.size() < 2) continue;
            int uid = stoi(p[0]);
            if (uid >= 0 && uid < (int)users.size()) users[uid]->notifications.push(p[1]);
        }
    }

    void loadPosts() {
        ifstream f(path("posts.txt"));
        string line;
        while (getline(f, line)) {
            if (line != "POSTSTART") continue;
            getline(f, line);
            auto head = splitPipe(line);
            if (head.size() < 3) break;
            int authorId = stoi(head[1]), likes = stoi(head[2]);

            string content;
            getline(f, content);

            getline(f, line);
            auto lb = splitPipe(line);
            unordered_set<int> likedBy;
            if (lb.size() >= 2 && !lb[1].empty()) {
                stringstream ss(lb[1]);
                string tok;
                while (getline(ss, tok, ',')) likedBy.insert(stoi(tok));
            }

            getline(f, line);
            auto cm = splitPipe(line);
            int n = cm.size() >= 2 ? stoi(cm[1]) : 0;
            vector<pair<int, string>> comments;
            for (int i = 0; i < n; i++) {
                getline(f, line);
                auto c = splitPipe(line);
                if (c.size() >= 2) comments.push_back({stoi(c[0]), c[1]});
            }
            getline(f, line);

            Post *p = posts.addPost(authorId, content);
            p->likes = likes;
            p->likedBy = likedBy;
            p->comments = comments;
        }
    }

    bool loadData() {
        if (!loadUsers()) return false;
        loadFriends();
        loadRequests();
        loadNotifications();
        loadPosts();
        return true;
    }

    void seedDemoData() {
        // Initial users
        int a = addUser("Anik Hasan", "anik", "123");
        int b = addUser("Bushra Akter", "bushra", "123");
        int c = addUser("Cawsar Hasan", "cawsar", "123");
        int d = addUser("Deya Jaman", "deya", "123");
        int e = addUser("Eve Sultana", "eve", "123");
        int r = addUser("Rahanuma Rashid", "rahanuma", "123");
        int bo = addUser("Bosudha Pal Suchana", "bosudha", "123");

        // Friend connections
        graph.addEdge(a, b);
        graph.addEdge(b, c);
        graph.addEdge(b, d);
        graph.addEdge(c, e);
        graph.addEdge(r, a);
        graph.addEdge(r, b);
        graph.addEdge(bo, r);
        graph.addEdge(bo, c);

        // Posts
        posts.addPost(b, "Data Structures lab was fun today!");
        posts.addPost(c, "Anyone up for a study group?");
        posts.addPost(a, "Hello everyone, this is my first post on FaceBook.");
        posts.addPost(r, "Excited to join FaceBook! Working on array and sorting algorithms.");
        posts.addPost(bo, "Graph traversal and BFS friend suggestions are working beautifully!");

        Post *p1 = posts.findById(1);
        if (p1) {
            p1->likes = 3;
            p1->likedBy = {a, c, r};
            p1->comments.push_back({a, "Totally agree!"});
            p1->comments.push_back({r, "Same here, graphs are amazing!"});
        }
        Post *p2 = posts.findById(2);
        if (p2) {
            p2->likes = 2;
            p2->likedBy = {b, bo};
            p2->comments.push_back({bo, "Yes, count me in!"});
        }
    }

    // Guarantees Rahanuma and Bosudha exist even when loading from existing data directory
    void ensureRequiredUsers() {
        bool updated = false;
        if (loginMap.find("rahanuma") == loginMap.end()) {
            int r = addUser("Rahanuma Rashid", "rahanuma", "123");
            if (users.size() > 1) graph.addEdge(r, 0);
            updated = true;
        }
        if (loginMap.find("bosudha") == loginMap.end()) {
            int bo = addUser("Bosudha Pal Suchana", "bosudha", "123");
            auto it = loginMap.find("rahanuma");
            if (it != loginMap.end()) graph.addEdge(bo, it->second);
            updated = true;
        }
        if (updated) saveData();
    }

public:
    FacebookApp() {
        if (!loadData()) {
            seedDemoData();
            saveData();
        } else {
            ensureRequiredUsers();
        }
    }

    ~FacebookApp() {
        saveData();
        for (User *u : users) delete u;
    }

    void run() {
        ui::initConsole();
        while (true) {
            ui::clearScreen();   // Autoclear console
            ui::drawHeader("F A C E B O O K");
            ui::box("WELCOME TO MINI FACEBOOK", ui::BOX_W, ui::FB_BLUE, true);
            cout << "\n";
            ui::box("1. LOGIN", 36, ui::FB_BLUE, true);
            cout << "\n";
            ui::box("2. REGISTER (NEW USER)", 36, ui::GREEN, true);
            cout << "\n";
            ui::box("3. EXIT", 36, ui::GRAY, true);
            cout << "\n";
            cout << "  " << ui::FB_BLUE << ui::BOLD << "Select an Option [1-3]: " << ui::RESET;

            int choice = readInt("");
            if (choice == 3) {
                saveData();
                ui::clearScreen();
                cout << "\n  Thank you for using FaceBook! Goodbye.\n\n";
                break;
            } else if (choice == 2) {
                registerFlow();
                // After registration, proceed directly to login flow
                loginFlow();
            } else if (choice == 1) {
                loginFlow();
            } else {
                cout << "  " << ui::RED << "Invalid option. Please choose 1, 2, or 3.\n" << ui::RESET;
                readLine("  Press Enter to continue...");
            }
        }
    }
};

int main() {
    FacebookApp app;
    app.run();
    return 0;
}
