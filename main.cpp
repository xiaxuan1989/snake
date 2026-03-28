#include <graphics.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <algorithm>
#include <cstdlib>
using namespace std;

// --- 全局常量 ---
enum FoodType { NORMAL, BONUS, PRIME, POISON }; // 食物类型
enum ObstaclesType { SOFT, HARD }; // 障碍类型
const int GRID_SIZE = 20;
const int WIDTH = 30 * GRID_SIZE;
const int HEIGHT = 30 * GRID_SIZE;
const int MAX_COLLISIONS = 5;

// --- 历史记录管理 ---
class HistoryManager {
private:
    string filename;

public:
    HistoryManager(const string& file) : filename(file) {}

    int getHighestScore(const string& version) {
        ifstream infile(filename);
        int maxScore = 0;

        if (infile.is_open()) {
            string line;
            while (getline(infile, line)) {
                if (line.find("Mode: " + version) != string::npos) {
                    size_t pos = line.rfind("Score: ");
                    if (pos != string::npos) {
                        int score = stoi(line.substr(pos + 7));
                        maxScore = max(maxScore, score);
                    }
                }
            }
            infile.close();
        }
        return maxScore;
    }

    void saveRecord(const string& version, const string& username, int score) {
        ofstream outfile(filename, ios::app); // 追加写入
        if (outfile.is_open()) {
            outfile << "Mode: " << version << " User: " << username << " Score: " << score << "\n";
            outfile.close();
        }
    }

    vector<string> search_user(const string& username) {
        ifstream infile(filename);
        if (infile.is_open()) {
            vector<string> records;
            string line;
            while (getline(infile, line)) {
                size_t user_pos = line.find("User: "); // 定位 "User: "
                if (user_pos != string::npos) {
                    user_pos += 6; // 跳过 "User: " 的长度
                    size_t space_pos = line.find(' ', user_pos); // 查找用户名结束位置
                    string user = line.substr(user_pos, space_pos - user_pos);
                    if (user == username) {
                        records.push_back(line); // 匹配成功，保存整条记录
                    }
                }
            }
            infile.close();
            return records;
        }
        cerr << "Error: Cannot open file for reading.\n";
        return {};
    }

    bool find_user(const string& username) {
        ifstream infile(filename);
        bool updated = false;
        if (infile.is_open()) {
            string line;
            while (getline(infile, line)) {
                if (line.find("User: " + username) != string::npos) {
                    updated = true;
                }
            }
            infile.close();
        }
        else {
            cout << "Unable to open history file!" << endl;
        }
        return updated;
    }

    bool modify_username(const string& old_username, const string& new_username) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open file for reading.\n";
            return false;
        }

        vector<string> lines; // 用于存储文件的每一行
        string line;
        bool modified = false;

        // 逐行读取文件内容
        while (getline(file, line)) {
            size_t user_pos = line.find("User: "); // 定位 "User: "
            if (user_pos != string::npos) {
                user_pos += 6; // 跳过 "User: " 的长度
                size_t space_pos = line.find(' ', user_pos); // 查找用户名结束位置
                string user = line.substr(user_pos, space_pos - user_pos);

                // 如果找到匹配的用户名，替换为新用户名
                if (user == old_username) {
                    line.replace(user_pos, user.length(), new_username);
                    modified = true;
                }
            }
            lines.push_back(line); // 保存行内容
        }
        file.close();

        // 如果没有任何修改，直接返回
        if (!modified) {
            cout << "No user found with the name: " << old_username << "\n";
            return false;
        }

        // 写回文件
        ofstream outfile(filename);
        if (!outfile.is_open()) {
            cerr << "Error: Cannot open file for writing.\n";
            return false;
        }

        for (const auto& modified_line : lines) {
            outfile << modified_line << "\n";
        }

        outfile.close();
        cout << "Username successfully updated from " << old_username << " to " << new_username << ".\n";
        return true;
    }

    void deleteRecord(const string& username, const string& version) {
        ifstream infile(filename);
        ofstream tempFile("temp.txt");
        bool deleted = false;

        if (infile.is_open() && tempFile.is_open()) {
            string line;
            while (getline(infile, line)) {
                if (line.find("User: " + username) != string::npos &&
                    line.find("Mode: " + version) != string::npos) {
                    deleted = true; // 不写入目标记录
                }
                else {
                    tempFile << line << "\n";
                }
            }
            infile.close();
            tempFile.close();
            remove(filename.c_str());
            rename("temp.txt", filename.c_str());

            if (!deleted) {
                cout << "No matching records were found, no deletion was performed." << endl;
            }
        }
        else {
            cout << "Unable to open history file for deletion!" << endl;
        }
    }
};

// --- 基础工具类 ---
class Point {
public:
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& p) const {
        return x == p.x && y == p.y;
    }
};

// --- 游戏实体基类 ---
class GameObject {
public:
    virtual void draw() const = 0;
    virtual ~GameObject() {}
};

// --- 蛇类 ---
class Snake : public GameObject {
private:
    vector<Point> body;
    char direction;
    COLORREF color;

public:
    Snake(Point start, char dir, COLORREF col) : direction(dir), color(col) {
        body.push_back(start);
    }
    char getDirection() const {
        return direction;
    }
    Point getNextHead() const {
        Point head = body.front(); // 当前蛇头位置
        switch (direction) {
        case 'W': case 72: head.y -= GRID_SIZE; break; // 'W' 或 上箭头
        case 'S': case 80: head.y += GRID_SIZE; break; // 'S' 或 下箭头
        case 'A': case 75: head.x -= GRID_SIZE; break; // 'A' 或 左箭头
        case 'D': case 77: head.x += GRID_SIZE; break; // 'D' 或 右箭头
        }
        return head;
    }
    void move() {
        Point nextHead = getNextHead();
        body.insert(body.begin(), nextHead);
        body.pop_back();
    }

    void grow() {
        body.push_back(body.back());
    }

    void changeDirection(char dir) {
        if ((direction == 'W' && dir != 'S') || (direction == 'S' && dir != 'W') ||
            (direction == 'A' && dir != 'D') || (direction == 'D' && dir != 'A') ||
            (direction == 72 && dir != 80) || (direction == 80 && dir != 72) ||
            (direction == 75 && dir != 77) || (direction == 77 && dir != 75)) {
            direction = dir;
        }
    }

    bool isCollision(Point p) {
        return find(body.begin(), body.end(), p) != body.end();
    }
    bool isSelfCollision() {
        Point nextHead = getNextHead();
        return count(body.begin() + 1, body.end(), nextHead) > 0;
    }

    Point getHead() const { return body.front(); }
    const vector<Point>& getBody() const { return body; }

    void draw() const override {
        for (const auto& segment : body) {
            setfillcolor(color);
            fillrectangle(segment.x, segment.y, segment.x + GRID_SIZE, segment.y + GRID_SIZE);
        }
    }
};

// --- 食物类 ---
class Food : public GameObject {
private:
    Point position;
    FoodType type;
    int score;

public:
    Food(Point pos, FoodType t) : position(pos), type(t) {
        if (type == NORMAL) score = 10;
        else if (type == BONUS) score = 20;
        else if (type == PRIME) score = 30;
        else if (type == POISON) score = -50;
    }

    Point getPos() const { return position; }
    FoodType getType() const { return type; }
    int getScore() const { return score; }

    void draw() const override {
        IMAGE imgFood;
        if (type == NORMAL) loadimage(&imgFood, _T("img/watme.png"));
        else if (type == BONUS) loadimage(&imgFood, _T("img/banana.png"));
        else if (type == PRIME) loadimage(&imgFood, _T("img/cake.png"));
        else if (type == POISON) loadimage(&imgFood, _T("img/poiAp.png"));
        putimage(position.x, position.y, &imgFood);
    }
};

// --- 游戏基类 ---
class SnakeGame {
protected:
    Snake* snake;
    vector<Food> food;
    int score;
    int highestScore; // 当前版本历史最高分
    bool gameOver;
    string mode;
    time_t startTime; // 记录游戏开始时间
    HistoryManager& historyManager; // 引用历史记录管理器

public:
    virtual bool checkSpaceAvailability() const {
        return food.size() + (snake ? snake->getBody().size() : 0) < (WIDTH / GRID_SIZE) * (HEIGHT / GRID_SIZE);
    }
    virtual Point randomPoint() {
        if (!checkSpaceAvailability()) {
            return Point(-1, -1);  // 返回无效点
        }
        Point p;
        do {
            p = Point(rand() % (WIDTH / GRID_SIZE) * GRID_SIZE,
                rand() % (HEIGHT / GRID_SIZE) * GRID_SIZE);
        } while ((snake && snake->isCollision(p)) ||
            find_if(food.begin(), food.end(), [&](const Food& f) { return f.getPos() == p; }) != food.end()); // find 改写，无需重载
        return p;
    }

    virtual void handleInput() {
        if (_kbhit()) {
            char key = _getch();
            if (key == 27) { // ESC 键 ASCII 值为 27
                gameOver = true;
                return;
            }
            if (key == -32) { // 检测方向键的特殊码
                key = _getch(); // 读取具体方向键码
            }
            if (key == 'W' || key == 'A' || key == 'S' || key == 'D' ||
                key == 'w' || key == 'a' || key == 's' || key == 'd' ||
                key == 72 || key == 75 || key == 80 || key == 77) { // 允许的按键
                snake->changeDirection(toupper(key));
            }
        }
    }

    int speed_time = 100;

    int getScore() const { return score; }
    int getSnakeLength() const { return snake ? snake->getBody().size() : 0; }

    SnakeGame(string mode, HistoryManager& hm)
        : mode(mode), historyManager(hm), snake(nullptr), score(0), gameOver(false) {
        srand((unsigned)time(nullptr));
        startTime = time(nullptr);
        highestScore = historyManager.getHighestScore(mode);
    }

    virtual ~SnakeGame() {
        delete snake;
    }

    virtual void handleCollision() = 0;
    virtual void updateGame() = 0;

    virtual void drawUI() {
        settextcolor(WHITE);
        settextstyle(20, 0, _T("Consolas"));

        // 当前时间
        time_t currentTime = time(nullptr);
        int elapsedTime = static_cast<int>(difftime(currentTime, startTime));
        if (score > highestScore) highestScore = score; // 更新最高分
        string ui[5] = { "MODE: " + mode, "LENGTH: " + to_string(getSnakeLength()),
                        "SCORE: " + to_string(score), "HIGH SCORE: " + to_string(highestScore),
                        "TIME: " + to_string(elapsedTime) + "s" };
        line(WIDTH, 0, WIDTH, HEIGHT);
        for (int i = 0; i < 5; ++i) {
            wstring wui(ui[i].begin(), ui[i].end());
            outtextxy(WIDTH + 20, (i + 1) * HEIGHT / 6, wui.c_str());
        }
    }

    virtual void run() {
        initgraph(WIDTH + 200, HEIGHT);
        setbkmode(TRANSPARENT); //双缓冲
        snake = new Snake(Point(WIDTH / 2, HEIGHT / 2), 'D', RGB(200, 200, 200));
        for (int i = 0; i < rand() % 5 + 1; ++i)
            food.push_back(Food(randomPoint(), NORMAL));
        // 开启双缓冲
        BeginBatchDraw();
        while (!gameOver) {
            cleardevice();
            setfillcolor(RGB(28, 115, 119));
            solidrectangle(0, 0, WIDTH, HEIGHT);
            drawUI();
            snake->draw();
            drawFood();
            handleInput();
            updateGame();
            FlushBatchDraw();  // 刷新到屏幕
            Sleep(speed_time);
        }
        EndBatchDraw();
        closegraph();
    }

    void drawFood() {
        for (const auto& f : food) f.draw();
    }
};

// --- 双人 ---
class DuelMode : public SnakeGame {
private:
    Snake* snake1; // 第一位玩家的蛇
    Snake* snake2; // 第二位玩家的蛇
    int score1, score2; // 两位玩家的分数

public:
    DuelMode(HistoryManager& hm)
        : SnakeGame("DUELM", hm), snake1(nullptr), snake2(nullptr), score1(0), score2(0) {}

    ~DuelMode() {
        delete snake1;
        delete snake2;
    }

    void handleInput() override {
        if (_kbhit()) {
            char key = _getch();
            if (key == 27) { // ESC 键退出
                gameOver = true;
                return;
            }
            if (key == -32) { // 检测方向键特殊码
                key = _getch(); // 读取具体方向键码
            }
            // 玩家1控制 WASD
            if (key == 'W' || key == 'A' || key == 'S' || key == 'D' ||
                key == 'w' || key == 'a' || key == 's' || key == 'd') {
                snake1->changeDirection(toupper(key));
            }
            // 玩家2控制方向键
            if (key == 72 || key == 75 || key == 80 || key == 77) {
                snake2->changeDirection(key);
            }
        }
    }

    void handleCollision() override {
        Point head1 = snake1->getNextHead();
        Point head2 = snake2->getNextHead();

        if (head1 == head2) {
            gameOver = true;
            return;
        }

        // 玩家1的碰撞检测
        if (snake1->isSelfCollision() ||
            snake2->isCollision(head1) ||
            head1.x < 0 || head1.x >= WIDTH ||
            head1.y < 0 || head1.y >= HEIGHT) {
            score1 -= 10; // 玩家1扣分
            delete snake1;
            Point startPos = randomPoint();
            if (startPos.x == -1 && startPos.y == -1) {
                gameOver = true;
                return; // 确保逻辑立即退出
            }
            snake1 = new Snake(startPos, 'D', RGB(200, 0, 0));
        }

        // 玩家2的碰撞检测
        if (snake2->isSelfCollision() ||
            snake1->isCollision(head2) ||
            head2.x < 0 || head2.x >= WIDTH ||
            head2.y < 0 || head2.y >= HEIGHT) {
            score2 -= 10; // 玩家2扣分
            delete snake2;
            Point startPos = randomPoint();
            if (startPos.x == -1 && startPos.y == -1) {
                gameOver = true;
                return; // 确保逻辑立即退出
            }
            snake2 = new Snake(startPos, 75, RGB(0, 0, 200));
        }
    }

    void updateGame() override {
        handleCollision();

        snake1->move();
        snake2->move();

        // 检测食物
        for (auto it = food.begin(); it != food.end();) {
            if (snake1->getHead() == it->getPos()) {
                score1 += it->getScore();
                snake1->grow();
                it = food.erase(it);
            }
            else if (snake2->getHead() == it->getPos()) {
                score2 += it->getScore();
                snake2->grow();
                it = food.erase(it);
            }
            else {
                ++it;
            }
        }

        // 如果食物吃光，生成新食物
        if (food.empty()) {
            for (int i = 0; i < rand() % 5 + 1; ++i)
                food.push_back(Food(randomPoint(), NORMAL));
        }
    }

    void run() override {
        initgraph(WIDTH + 200, HEIGHT);
        setbkmode(TRANSPARENT); // 双缓冲

        // 初始化两条蛇
        snake1 = new Snake(Point(WIDTH / 2 - 10 * GRID_SIZE, HEIGHT / 2), 'D', RGB(200, 0, 0));
        snake2 = new Snake(Point(WIDTH / 2 + 10 * GRID_SIZE, HEIGHT / 2), 75, RGB(0, 0, 200)); // 玩家2起始方向左

        for (int i = 0; i < rand() % 5 + 1; ++i)
            food.push_back(Food(randomPoint(), NORMAL));

        BeginBatchDraw();
        while (!gameOver) {
            cleardevice();
            setfillcolor(RGB(28, 115, 119));
            solidrectangle(0, 0, WIDTH, HEIGHT);
            drawUI();
            snake1->draw();
            snake2->draw();
            drawFood();
            handleInput();
            updateGame();
            FlushBatchDraw();  // 刷新到屏幕
            Sleep(speed_time);
        }
        EndBatchDraw();
        closegraph();
    }

    void drawUI() override {
        settextcolor(WHITE);
        settextstyle(20, 0, _T("Consolas"));

        time_t currentTime = time(nullptr);
        int elapsedTime = static_cast<int>(difftime(currentTime, startTime));

        string ui[6] = { "MODE: " + mode,
                         "PLAYER1 SCORE: " + to_string(score1),
                         "PLAYER2 SCORE: " + to_string(score2),
                         "PLAYER1 LENGTH: " + to_string(snake1->getBody().size()),
                         "PLAYER2 LENGTH: " + to_string(snake2->getBody().size()),
                         "TIME: " + to_string(elapsedTime) + "s" };
        line(WIDTH, 0, WIDTH, HEIGHT);
        for (int i = 0; i < 6; ++i) {
            wstring wui(ui[i].begin(), ui[i].end());
            outtextxy(WIDTH + 20, (i + 1) * HEIGHT / 7, wui.c_str());
        }
    }
};

// --- 人机 ---
class AiSnake : public Snake {
public:
    AiSnake(Point startPos, char initDirection, COLORREF color)
        : Snake(startPos, initDirection, color) {}

    void decideDirection(const Snake* playerSnake, const vector<Food>& food) {
        Point aiHead = getHead();
        char currentDirection = getDirection();

        // 计算各个行为的优先级
        double attackWeight = 1.5;
        double foodWeight = 1.0;
        double avoidWeight = 2.0;

        // 避免自我碰撞的优先级最高
        if (isSelfCollision()) {
            char newDirection = findSafeDirection(currentDirection, aiHead);
            if (newDirection != currentDirection) {
                changeDirection(newDirection);
                return;
            }
        }

        // 进攻玩家的策略
        Point playerNextHead = predictNextHead(playerSnake);
        if (distance(aiHead, playerNextHead) <= GRID_SIZE * 3) {
            changeDirectionToBlockPlayer(playerNextHead, aiHead, currentDirection);
            return;
        }

        // 寻找最近的食物
        Point closestFood = food[0].getPos();
        for (const auto& f : food) {
            if (distance(f.getPos(), aiHead) < distance(closestFood, aiHead)) {
                closestFood = f.getPos();
            }
        }

        // 向最近的食物方向移动
        changeDirectionToFood(closestFood, aiHead, currentDirection);
    }

private:
    Point predictNextHead(const Snake* playerSnake) {
        Point head = playerSnake->getHead();
        char direction = playerSnake->getDirection();
        switch (direction) {
        case 'W': head.y -= GRID_SIZE; break;
        case 'A': head.x -= GRID_SIZE; break;
        case 'S': head.y += GRID_SIZE; break;
        case 'D': head.x += GRID_SIZE; break;
        }
        return head;
    }

    double distance(const Point& p1, const Point& p2) {
        return abs(p1.x - p2.x) + abs(p1.y - p2.y);
    }

    // 根据安全性找到安全方向
    char findSafeDirection(char currentDirection, const Point& aiHead) {
        if (currentDirection == 'W' && !isSelfCollisionAt(aiHead.x, aiHead.y - GRID_SIZE)) return 'W';
        if (currentDirection == 'S' && !isSelfCollisionAt(aiHead.x, aiHead.y + GRID_SIZE)) return 'S';
        if (currentDirection == 'A' && !isSelfCollisionAt(aiHead.x - GRID_SIZE, aiHead.y)) return 'A';
        if (currentDirection == 'D' && !isSelfCollisionAt(aiHead.x + GRID_SIZE, aiHead.y)) return 'D';

        // 选择一个安全方向
        if (currentDirection != 'W' && !isSelfCollisionAt(aiHead.x, aiHead.y - GRID_SIZE)) return 'W';
        if (currentDirection != 'S' && !isSelfCollisionAt(aiHead.x, aiHead.y + GRID_SIZE)) return 'S';
        if (currentDirection != 'A' && !isSelfCollisionAt(aiHead.x - GRID_SIZE, aiHead.y)) return 'A';
        if (currentDirection != 'D' && !isSelfCollisionAt(aiHead.x + GRID_SIZE, aiHead.y)) return 'D';

        return currentDirection;
    }

    bool isSelfCollisionAt(int x, int y) {
        return isCollision(Point(x, y));
    }

    // 根据玩家位置尝试阻挡玩家
    void changeDirectionToBlockPlayer(const Point& playerNextHead, const Point& aiHead, char currentDirection) {
        if (playerNextHead.x > aiHead.x && currentDirection != 'A') {
            changeDirection('D'); // 向右
        }
        else if (playerNextHead.x < aiHead.x && currentDirection != 'D') {
            changeDirection('A'); // 向左
        }
        else if (playerNextHead.y > aiHead.y && currentDirection != 'W') {
            changeDirection('S'); // 向下
        }
        else if (playerNextHead.y < aiHead.y && currentDirection != 'S') {
            changeDirection('W'); // 向上
        }
    }

    // 根据食物位置改变方向
    void changeDirectionToFood(const Point& closestFood, const Point& aiHead, char currentDirection) {
        if (aiHead.x < closestFood.x && currentDirection != 'A') {
            changeDirection('D');
        }
        else if (aiHead.x > closestFood.x && currentDirection != 'D') {
            changeDirection('A');
        }
        else if (aiHead.y < closestFood.y && currentDirection != 'W') {
            changeDirection('S');
        }
        else if (aiHead.y > closestFood.y && currentDirection != 'S') {
            changeDirection('W');
        }
    }
};

class AIMode : public SnakeGame {
private:
    Snake* playerSnake;  // 玩家控制的蛇
    AiSnake* aiSnake;    // AI 控制的蛇
    int playerScore, aiScore;

public:
    AIMode(HistoryManager& hm)
        : SnakeGame("AIMOD", hm), playerSnake(nullptr), aiSnake(nullptr), playerScore(0), aiScore(0) {}

    ~AIMode() {
        delete playerSnake;
        delete aiSnake;
    }

    void handleInput() override {
        if (_kbhit()) {
            char key = _getch();
            if (key == 27) { // ESC 键退出
                gameOver = true;
                return;
            }
            if (key == -32) { // 检测方向键特殊码
                key = _getch();
            }
            // 玩家控制 WASD
            if (key == 'W' || key == 'A' || key == 'S' || key == 'D' ||
                key == 'w' || key == 'a' || key == 's' || key == 'd') {
                playerSnake->changeDirection(toupper(key));
            }
        }
    }

    void handleCollision() override {
        Point playerHead = playerSnake->getNextHead();
        Point aiHead = aiSnake->getNextHead();

        if (playerHead == aiHead) {
            gameOver = true;
            return;
        }

        // 玩家蛇碰撞检测
        if (playerSnake->isSelfCollision() ||
            aiSnake->isCollision(playerHead) ||
            playerHead.x < 0 || playerHead.x >= WIDTH ||
            playerHead.y < 0 || playerHead.y >= HEIGHT) {
            playerScore -= 10;
            delete playerSnake;
            Point startPos = randomPoint();
            playerSnake = new Snake(startPos, 'D', RGB(200, 0, 0));
        }

        // AI 蛇碰撞检测
        if (aiSnake->isSelfCollision() ||
            playerSnake->isCollision(aiHead) ||
            aiHead.x < 0 || aiHead.x >= WIDTH ||
            aiHead.y < 0 || aiHead.y >= HEIGHT) {
            aiScore -= 10;
            delete aiSnake;
            Point startPos = randomPoint();
            aiSnake = new AiSnake(startPos, 'D', RGB(0, 0, 200));
        }
    }

    void updateGame() override {
        handleCollision();

        // AI 决策
        aiSnake->decideDirection(playerSnake, food);

        playerSnake->move();
        aiSnake->move();

        // 检测食物
        for (auto it = food.begin(); it != food.end();) {
            if (playerSnake->getHead() == it->getPos()) {
                playerScore += it->getScore();
                playerSnake->grow();
                it = food.erase(it);
            }
            else if (aiSnake->getHead() == it->getPos()) {
                aiScore += it->getScore();
                aiSnake->grow();
                it = food.erase(it);
            }
            else {
                ++it;
            }
        }

        // 如果食物吃光，生成新食物
        if (food.empty()) {
            for (int i = 0; i < rand() % 5 + 1; ++i)
                food.push_back(Food(randomPoint(), NORMAL));
        }
    }

    void run() override {
        initgraph(WIDTH + 200, HEIGHT);
        setbkmode(TRANSPARENT); // 双缓冲

        // 初始化玩家蛇和 AI 蛇
        playerSnake = new Snake(Point(WIDTH / 2 - 10 * GRID_SIZE, HEIGHT / 2), 'D', RGB(200, 0, 0));
        aiSnake = new AiSnake(Point(WIDTH / 2 + 10 * GRID_SIZE, HEIGHT / 2), 'A', RGB(0, 0, 200));

        for (int i = 0; i < rand() % 5 + 1; ++i)
            food.push_back(Food(randomPoint(), NORMAL));

        BeginBatchDraw();
        while (!gameOver) {
            cleardevice();
            setfillcolor(RGB(28, 115, 119));
            solidrectangle(0, 0, WIDTH, HEIGHT);
            drawUI();
            playerSnake->draw();
            aiSnake->draw();
            drawFood();
            handleInput();
            updateGame();
            FlushBatchDraw();
            Sleep(speed_time);
        }
        EndBatchDraw();
        closegraph();
    }

    void drawUI() override {
        settextcolor(WHITE);
        settextstyle(20, 0, _T("Consolas"));

        time_t currentTime = time(nullptr);
        int elapsedTime = static_cast<int>(difftime(currentTime, startTime));

        string ui[6] = { "MODE: " + mode,
                         "PLAYER SCORE: " + to_string(playerScore),
                         "AI SCORE: " + to_string(aiScore),
                         "PLAYER LENGTH: " + to_string(playerSnake->getBody().size()),
                         "AI LENGTH: " + to_string(aiSnake->getBody().size()),
                         "TIME: " + to_string(elapsedTime) + "s" };
        line(WIDTH, 0, WIDTH, HEIGHT);
        for (int i = 0; i < 6; ++i) {
            wstring wui(ui[i].begin(), ui[i].end());
            outtextxy(WIDTH + 20, (i + 1) * HEIGHT / 7, wui.c_str());
        }
    }
};

class DoubleAIMode : public SnakeGame {
private:
    Snake* playerSnake;  // 玩家控制的蛇
    AiSnake* aiSnake1;   // 第一条AI蛇
    AiSnake* aiSnake2;   // 第二条AI蛇
    int playerScore, aiScore1, aiScore2;

public:
    DoubleAIMode(HistoryManager& hm)
        : SnakeGame("DOUAI", hm), playerSnake(nullptr), aiSnake1(nullptr), aiSnake2(nullptr), playerScore(0), aiScore1(0), aiScore2(0) {}

    ~DoubleAIMode() {
        delete playerSnake;
        delete aiSnake1;
        delete aiSnake2;
    }

    void handleInput() override {
        if (_kbhit()) {
            char key = _getch();
            if (key == 27) { // ESC 键退出
                gameOver = true;
                return;
            }
            if (key == -32) { // 检测方向键特殊码
                key = _getch();
            }
            // 玩家控制 WASD
            if (key == 'W' || key == 'A' || key == 'S' || key == 'D' ||
                key == 'w' || key == 'a' || key == 's' || key == 'd') {
                playerSnake->changeDirection(toupper(key));
            }
        }
    }

    void handleCollision() override {
        Point playerHead = playerSnake->getNextHead();
        Point aiHead1 = aiSnake1->getNextHead();
        Point aiHead2 = aiSnake2->getNextHead();

        // 玩家与AI碰撞检测
        if (playerHead == aiHead1 || playerHead == aiHead2) {
            gameOver = true;
            return;
        }

        // 玩家蛇碰撞检测
        if (playerSnake->isSelfCollision() ||
            aiSnake1->isCollision(playerHead) ||
            aiSnake2->isCollision(playerHead) ||
            playerHead.x < 0 || playerHead.x >= WIDTH ||
            playerHead.y < 0 || playerHead.y >= HEIGHT) {
            playerScore -= 10;
            delete playerSnake;
            Point startPos = randomPoint();
            playerSnake = new Snake(startPos, 'D', RGB(200, 0, 0));
        }

        // 第一条AI蛇碰撞检测
        if (aiSnake1->isSelfCollision() ||
            playerSnake->isCollision(aiHead1) ||
            aiSnake2->isCollision(aiHead1) ||
            aiHead1.x < 0 || aiHead1.x >= WIDTH ||
            aiHead1.y < 0 || aiHead1.y >= HEIGHT) {
            aiScore1 -= 10;
            delete aiSnake1;
            Point startPos = randomPoint();
            aiSnake1 = new AiSnake(startPos, 'D', RGB(0, 0, 200));
        }

        // 第二条AI蛇碰撞检测
        if (aiSnake2->isSelfCollision() ||
            playerSnake->isCollision(aiHead2) ||
            aiSnake1->isCollision(aiHead2) ||
            aiHead2.x < 0 || aiHead2.x >= WIDTH ||
            aiHead2.y < 0 || aiHead2.y >= HEIGHT) {
            aiScore2 -= 10;
            delete aiSnake2;
            Point startPos = randomPoint();
            aiSnake2 = new AiSnake(startPos, 'D', RGB(0, 200, 0));
        }
    }

    void updateGame() override {
        handleCollision();

        // AI决策
        aiSnake1->decideDirection(playerSnake, food);
        aiSnake2->decideDirection(playerSnake, food);

        playerSnake->move();
        aiSnake1->move();
        aiSnake2->move();

        // 检测食物
        for (auto it = food.begin(); it != food.end();) {
            if (playerSnake->getHead() == it->getPos()) {
                playerScore += it->getScore();
                playerSnake->grow();
                it = food.erase(it);
            }
            else if (aiSnake1->getHead() == it->getPos()) {
                aiScore1 += it->getScore();
                aiSnake1->grow();
                it = food.erase(it);
            }
            else if (aiSnake2->getHead() == it->getPos()) {
                aiScore2 += it->getScore();
                aiSnake2->grow();
                it = food.erase(it);
            }
            else {
                ++it;
            }
        }

        // 如果食物吃光，生成新食物
        if (food.empty()) {
            for (int i = 0; i < rand() % 5 + 1; ++i)
                food.push_back(Food(randomPoint(), NORMAL));
        }
    }

    void run() override {
        initgraph(WIDTH + 200, HEIGHT);
        setbkmode(TRANSPARENT); // 双缓冲

        // 初始化玩家蛇和两条AI蛇
        playerSnake = new Snake(Point(WIDTH / 2 - 10 * GRID_SIZE, HEIGHT / 2), 'D', RGB(200, 0, 0));
        aiSnake1 = new AiSnake(Point(WIDTH / 2 + 10 * GRID_SIZE, HEIGHT / 2), 'A', RGB(0, 0, 200));
        aiSnake2 = new AiSnake(Point(WIDTH / 2 + 20 * GRID_SIZE, HEIGHT / 2), 'S', RGB(0, 200, 0));

        for (int i = 0; i < rand() % 5 + 1; ++i)
            food.push_back(Food(randomPoint(), NORMAL));

        BeginBatchDraw();
        while (!gameOver) {
            cleardevice();
            setfillcolor(RGB(28, 115, 119));
            solidrectangle(0, 0, WIDTH, HEIGHT);
            drawUI();
            playerSnake->draw();
            aiSnake1->draw();
            aiSnake2->draw();
            drawFood();
            handleInput();
            updateGame();
            FlushBatchDraw();
            Sleep(speed_time);
        }
        EndBatchDraw();
        closegraph();
    }

    void drawUI() override {
        settextcolor(WHITE);
        settextstyle(20, 0, _T("Consolas"));

        time_t currentTime = time(nullptr);
        int elapsedTime = static_cast<int>(difftime(currentTime, startTime));

        string ui[8] = { "MODE: " + mode,
                         "PLAYER SCORE: " + to_string(playerScore),
                         "AI 1 SCORE: " + to_string(aiScore1),
                         "AI 2 SCORE: " + to_string(aiScore2),
                         "PLAYER LENGTH: " + to_string(playerSnake->getBody().size()),
                         "AI 1 LENGTH: " + to_string(aiSnake1->getBody().size()),
                         "AI 2 LENGTH: " + to_string(aiSnake2->getBody().size()),
                         "TIME: " + to_string(elapsedTime) + "s" };
        line(WIDTH, 0, WIDTH, HEIGHT);
        for (int i = 0; i < 7; ++i) {
            wstring wui(ui[i].begin(), ui[i].end());
            outtextxy(WIDTH + 20, (i + 1) * HEIGHT / 8, wui.c_str());
        }
    }
};

// --- 入门 ---
class BeginnerMode : public SnakeGame {
public:
    BeginnerMode(HistoryManager& hm) : SnakeGame("BEGIN", hm) {}

    int rectWidthNum = 2 + rand() % (WIDTH / GRID_SIZE - 5);
    int rectHeightNum = 2 + rand() % (HEIGHT / GRID_SIZE - 5);
    int rectWidth = (rectWidthNum) * GRID_SIZE;  // 随机宽度
    int rectHeight = (rectHeightNum) * GRID_SIZE; // 随机高度

    int x1 = (rand() % (WIDTH / GRID_SIZE - rectWidthNum)) * GRID_SIZE;  // 随机左上角坐标
    int y1 = (rand() % (WIDTH / GRID_SIZE - rectHeightNum)) * GRID_SIZE;
    int x2 = x1 + rectWidth;  // 计算右下角坐标
    int y2 = y1 + rectHeight;

    bool isPointInZone(Point p) {
        return (p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2);
    }

    void handleCollision() override { gameOver = true; }

    void generateFood() {
        Point foodPos = randomPoint();
        if (foodPos.x != -1 && foodPos.y != -1) {
            int randValue = rand() % 10;
            FoodType type = randValue < 7 ? NORMAL : (randValue < 9 ? BONUS : PRIME);
            food.push_back(Food(foodPos, type));
        }
    }

    void updateGame() override {
        Point nextHead = snake->getNextHead();
        // 碰撞检测
        if (nextHead.x < 0 || nextHead.x >= WIDTH || nextHead.y < 0 || nextHead.y >= HEIGHT ||
            snake->isSelfCollision()) {
            handleCollision();
            return; // 碰撞后直接退出本轮更新
        }
        snake->move();
        Point head = nextHead;

        if (isPointInZone(head)) {
            speed_time = 50; // 蛇头在矩形区域内时加速
        }
        else {
            speed_time = 100; // 蛇头在矩形区域外时恢复默认速度
        }

        // 食物检测
        for (auto it = food.begin(); it != food.end(); ++it) {
            if (head == it->getPos()) {
                score += it->getScore();
                snake->grow();
                it = food.erase(it);
                break;
            }
        }

        // 如果食物吃光，生成新食物
        if (food.empty()) {
            rectWidthNum = 2 + rand() % (WIDTH / GRID_SIZE - 5);
            rectHeightNum = 2 + rand() % (HEIGHT / GRID_SIZE - 5);
            rectWidth = (rectWidthNum)*GRID_SIZE;  // 随机宽度
            rectHeight = (rectHeightNum)*GRID_SIZE; // 随机高度

            x1 = (rand() % (WIDTH / GRID_SIZE - rectWidthNum)) * GRID_SIZE;  // 随机左上角坐标
            y1 = (rand() % (WIDTH / GRID_SIZE - rectHeightNum)) * GRID_SIZE;
            x2 = x1 + rectWidth;  // 计算右下角坐标
            y2 = y1 + rectHeight;
            for (int i = 0; i < rand() % 5 + 1; ++i) generateFood();
        }
    }

    void run() override {
        initgraph(WIDTH + 200, HEIGHT);
        setbkmode(TRANSPARENT); //双缓冲
        snake = new Snake(Point(WIDTH / 2, HEIGHT / 2), 'D', RGB(200, 200, 200));
        for (int i = 0; i < rand() % 5 + 1; ++i)
            food.push_back(Food(randomPoint(), NORMAL));
        // 开启双缓冲
        BeginBatchDraw();
        while (!gameOver) {
            cleardevice();
            setfillcolor(RGB(28, 115, 119));
            solidrectangle(0, 0, WIDTH, HEIGHT);
            setfillcolor(RGB(100, 200, 255)); // 例如天蓝色
            solidrectangle(x1, y1, x2, y2);
            drawUI();
            snake->draw();
            drawFood();
            handleInput();
            updateGame();
            FlushBatchDraw();  // 刷新到屏幕
            Sleep(speed_time);
        }
        EndBatchDraw();
        closegraph();
    }

    void drawUI() override {
        settextcolor(WHITE);
        settextstyle(20, 0, _T("Consolas"));
        // 当前时间
        time_t currentTime = time(nullptr);
        int elapsedTime = static_cast<int>(difftime(currentTime, startTime));
        if (score > highestScore) highestScore = score; // 更新最高分
        string ui[5] = { "MODE: " + mode, "LENGTH: " + to_string(getSnakeLength()),
                        "SCORE: " + to_string(score), "HIGH SCORE: " + to_string(highestScore),
                        "TIME: " + to_string(elapsedTime) + "s" };
        line(WIDTH, 0, WIDTH, HEIGHT);
        for (int i = 0; i < 5; ++i) {
            wstring wui(ui[i].begin(), ui[i].end());
            outtextxy(WIDTH + 20, (i + 1) * HEIGHT / 6, wui.c_str());
        }
    }
};

// --- 进阶 ---
class Obstacles : public GameObject {
private:
    Point position;
    ObstaclesType type;
    int score;

public:
    Obstacles(Point pos, ObstaclesType t) : position(pos), type(t) {
        if (type == SOFT) score = -10;
        else if (type == HARD) score = -20;
    }

    Point getPos() const { return position; }
    int getScore() const { return score; }

    void draw() const override {
        if (type == SOFT) {
            setfillcolor(RGB(100, 100, 100));
            solidrectangle(position.x, position.y,
                position.x + GRID_SIZE, position.y + GRID_SIZE);
        }
        else {
            IMAGE imgFood;
            loadimage(&imgFood, _T("img/HARD.png"));
            putimage(position.x, position.y, &imgFood);
        }
    }
};
class IntermediateMode : public SnakeGame {
public:
    vector<Obstacles> obstacles;
    IntermediateMode(HistoryManager& hm) : SnakeGame("INTER", hm) {}

    bool checkSpaceAvailability() const override {
        return obstacles.size() + food.size() + (snake ? snake->getBody().size() : 0) < (WIDTH / GRID_SIZE) * (HEIGHT / GRID_SIZE);
    }
    Point randomPoint() override {
        if (!checkSpaceAvailability()) {
            return Point(-1, -1);  // 返回无效点
        }
        Point p;
        do {
            p = Point(rand() % (WIDTH / GRID_SIZE) * GRID_SIZE,
                rand() % (HEIGHT / GRID_SIZE) * GRID_SIZE);
        } while ((snake && snake->isCollision(p)) ||
            find_if(food.begin(), food.end(), [&](const Food& f) { return f.getPos() == p; }) != food.end() ||
            find_if(obstacles.begin(), obstacles.end(), [&](const Obstacles& o) { return o.getPos() == p; }) != obstacles.end()); // find 改写，无需重载
        return p;
    }

    void drawObstacles() {
        for (const auto& obs : obstacles) obs.draw();
    }

    void run() override {
        initgraph(WIDTH + 200, HEIGHT);
        setbkmode(TRANSPARENT); //双缓冲
        snake = new Snake(Point(WIDTH / 2, HEIGHT / 2), 'D', RGB(200, 200, 200));
        for (int i = 0; i < rand() % 5 + 1; ++i)
            food.push_back(Food(randomPoint(), NORMAL));
        // 开启双缓冲
        BeginBatchDraw();
        while (!gameOver) {
            cleardevice();
            setfillcolor(RGB(28, 115, 119));
            solidrectangle(0, 0, WIDTH, HEIGHT);
            drawUI();
            snake->draw();
            drawFood();
            drawObstacles();
            handleInput();
            updateGame();
            FlushBatchDraw();  // 刷新到屏幕
            Sleep(speed_time);
        }
        EndBatchDraw();
        closegraph();
    }

    void generateFood() {
        Point foodPos = randomPoint();
        if (foodPos.x != -1 && foodPos.y != -1) {
            int randValue = rand() % 100;
            FoodType type = randValue < 70 ? NORMAL : (randValue < 95 ? BONUS : POISON);
            food.push_back(Food(foodPos, type));
        }
    }

    void handleCollision() override {
        for (const auto& segment : snake->getBody()) {
            int randValue = rand() % 10;
            ObstaclesType type = randValue < 7 ? SOFT : HARD;
            obstacles.push_back(Obstacles(segment, type));
        }
        delete snake;
        snake = nullptr;
        Point startPos = randomPoint();
        if (startPos.x == -1 && startPos.y == -1) {
            gameOver = true;
            return; // 确保逻辑立即退出
        }
        snake = new Snake(startPos, 'D', RGB(200, 200, 200));
        for (int i = 0; i < rand() % 5 + 1; ++i) generateFood();
    }

    void updateGame() override {
        Point nextHead = snake->getNextHead();
        // 碰撞检测
        if (nextHead.x < 0 || nextHead.x >= WIDTH || nextHead.y < 0 || nextHead.y >= HEIGHT ||
            snake->isSelfCollision()) {
            handleCollision();
            return; // 碰撞后直接退出本轮更新
        }
        for (auto it = obstacles.begin(); it != obstacles.end(); ++it) {
            if (nextHead == it->getPos()) {
                score += it->getScore();
                handleCollision();
                return; // 碰撞后直接退出本轮更新
            }
        }
        snake->move();
        Point head = nextHead;

        // 食物检测
        for (auto it = food.begin(); it != food.end(); ++it) {
            if (head == it->getPos()) {
                score += it->getScore();
                snake->grow();
                it = food.erase(it);
                break;
            }
        }

        if (food.empty()) {
            for (int i = 0; i < rand() % 5 + 1; ++i) generateFood();
        }
    }


};

// --- 高级 ---
class AdvancedMode : public SnakeGame {
private:
    int collisions; // 碰撞次数
    int lives = MAX_COLLISIONS;  // 剩余生命数，仅适用于特定模式

public:
    int getLives() const { return lives; }
    AdvancedMode(HistoryManager& hm) : SnakeGame("ADVAN", hm), collisions(0) {}

    void generateFood() {
        Point foodPos = randomPoint();
        if (foodPos.x != -1 && foodPos.y != -1) {
            int randValue = rand() % 10;
            FoodType type = randValue < 7 ? NORMAL : (randValue < 9 ? BONUS : PRIME);
            food.push_back(Food(foodPos, type));
        }
    }

    void handleCollision() override {
        collisions++;
        for (const auto& segment : snake->getBody()) {
            food.push_back(Food(segment, NORMAL));
        }
        delete snake;
        snake = nullptr; // 防止异常退出
        if (collisions > MAX_COLLISIONS) {
            gameOver = true;
            return; // 确保逻辑立即退出
        }
        Point startPos = randomPoint();
        if (startPos.x == -1 && startPos.y == -1) {
            gameOver = true;
            return; // 确保逻辑立即退出
        }
        snake = new Snake(startPos, 'D', RGB(200, 200, 200));
        for (int i = 0; i < rand() % 5 + 1; ++i) generateFood();
    }

    void updateGame() override {
        Point nextHead = snake->getNextHead();
        // 碰撞检测
        if (nextHead.x < 0 || nextHead.x >= WIDTH || nextHead.y < 0 || nextHead.y >= HEIGHT) {
            lives--;
            score -= 50;  // 撞墙扣分
            handleCollision();
            return; // 碰撞后直接退出本轮更新
        }
        if (snake->isSelfCollision()) {
            lives--;
            score -= 30;  // 自撞扣分
            handleCollision();
            return; // 碰撞后直接退出本轮更新
        }
        snake->move();
        Point head = nextHead;

        // 食物检测
        for (auto it = food.begin(); it != food.end(); ++it) {
            if (head == it->getPos()) {
                switch (it->getType()) {
                case NORMAL:
                    score += 20;
                    break;
                case BONUS:
                    score += 50;
                    break;
                case PRIME:
                    score += 60;
                    break;
                case POISON:
                    score -= 100;
                    break;
                }
                snake->grow();
                it = food.erase(it);
                break;
            }
        }

        if (food.empty()) {
            for (int i = 0; i < rand() % 5 + 1; ++i) generateFood();
        }
    }

    void drawUI() override {
        settextcolor(WHITE);
        settextstyle(20, 0, _T("Consolas"));

        // 当前时间
        time_t currentTime = time(nullptr);
        int elapsedTime = static_cast<int>(difftime(currentTime, startTime));
        if (score > highestScore) highestScore = score; // 更新最高分
        string ui[6] = { "MODE: " + mode, "LENGTH: " + to_string(getSnakeLength()),
                        "SCORE: " + to_string(score), "HIGH SCORE: " + to_string(highestScore),
                        "TIME: " + to_string(elapsedTime) + "s", "LIVES: " + to_string(lives) };
        line(WIDTH, 0, WIDTH, HEIGHT);
        for (int i = 0; i < 6; ++i) {
            wstring wui(ui[i].begin(), ui[i].end());
            outtextxy(WIDTH + 20, (i + 1) * HEIGHT / 7, wui.c_str());
        }
    }
};

// --- 主函数 ---
int main() {
    HistoryManager historyManager("history.txt");
    cout << "Enter your username: ";
    string username;
    cin >> username;
    while (1) {
        system("cls");
        cout << "SNAKE GAME\n";
        cout << "User: " << username << '\n';
        cout << "1. Start\n";
        cout << "2. History\n";
        cout << "0. Exit\n";
        cout << "Select: ";
        int choice;
        cin >> choice;
        if (choice == 1) {
            while (1) {
                system("cls");
                cout << "Mode: \n";
                cout << "1. Beginner\n";
                cout << "2. Intermediate\n";
                cout << "3. Advanced\n";
                cout << "4. Duel\n";
                cout << "5. Solo AI\n";
                cout << "6. Double AI\n";
                cout << "0. Return\n";
                cout << "Select: ";
                int gameChoice;
                cin >> gameChoice;

                SnakeGame* game = nullptr;
                string mode;

                if (gameChoice == 1) {
                    game = new BeginnerMode(historyManager);
                    mode = "BEGIN";
                }
                else if (gameChoice == 2) {
                    game = new IntermediateMode(historyManager);
                    mode = "INTER";
                }
                else if (gameChoice == 3) {
                    game = new AdvancedMode(historyManager);
                    mode = "ADVAN";
                }
                else if (gameChoice == 4) {
                    game = new DuelMode(historyManager);
                    mode = "DUELM";
                }
                else if (gameChoice == 5) {
                    game = new AIMode(historyManager);
                    mode = "AIMOD";
                }
                else if (gameChoice == 6) {
                    game = new DoubleAIMode(historyManager);
                    mode = "DOUAI";
                }
                else break;

                game->run();
                int score = game->getScore();
                historyManager.saveRecord(mode, username, score);
                delete game;

                system("cls");
                cout << "GAME OVER, press any key...";
                system("pause>nul");
            }
        }
        else if (choice == 2) {
            while (1) {
                system("cls");
                cout << "History: \n";
                cout << "1. View History\n";
                cout << "2. Modify Username\n";
                cout << "3. Delete History\n";
                cout << "0. Return\n";
                cout << "Select: ";
                int history_c;
                cin >> history_c;
                system("cls");
                if (history_c == 1) {
                    cout << "View History\n";
                    vector<string> records = historyManager.search_user(username);
                    if (!records.empty()) {
                        cout << "Records:\n";
                        for (const auto& record : records) {
                            cout << record << "\n";
                        }
                    }
                    else cout << "No records found.\n";
                    system("pause");
                }
                else if (history_c == 2) {
                    cout << "Modify Username\n";
                    if (historyManager.find_user(username)) {
                        cout << "Enter new username: ";
                        string newUsername;
                        cin >> newUsername;
                        historyManager.modify_username(username, newUsername);
                    }
                    else {
                        cout << "No records found.\n";
                        system("pause");
                    }
                }
                else if (history_c == 3) {
                    cout << "Delete History\n";
                    cout << "Enter a name: ";
                    string name;
                    cin >> name;
                    cout << "Enter a mode: ";
                    string version;
                    cin >> version;
                    historyManager.deleteRecord(name, version);
                    system("pause");
                }
                else break;
            }
        }
        else break;
    }
    return 0;
}