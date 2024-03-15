#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <map>
#include <fstream>
#include <ctime>
#include <utility>
#include <cstdlib>
#include <cassert>

using namespace std;
const int M_SIZE = 200;
const int B_SIZE = 10;
const int BOAT_SIZE = 5;


char mp[M_SIZE][M_SIZE];
int boat_capacity;
int money,frame_id;
int k;

// queue<pair<int, int>> goods_queue;

int dx[4] = {0, 1, -1, 0};
int dy[4] = {-1, 0, 0, 1};
string dir[4] = {"1", "3", "2", "0"};
bool has_cracked[10];
ofstream fout("out.txt");
int choice[10] = {0,1,2,3,4,5,6,7,8,9}; //机器人的泊位选择
map<pair<int,int>,int> berthid;   //泊位坐标到id的映射

bool isValid(int x, int y) {
    return x >= 0 && x < M_SIZE && y >= 0 && y < M_SIZE && mp[x][y] != '#' && mp[x][y] != '*';
}

vector<string> BFS(int startX, int startY, int endX, int endY) {
    queue<pair<int, int> > q;
    vector<vector<bool> > visited(M_SIZE, vector<bool>(M_SIZE, false));
    map<pair<int, int>, pair<int, int> > parent;
    map<pair<int, int>, string> move;

    q.push({startX, startY});
    visited[startX][startY] = true;

    while (!q.empty()) {
        auto [x,y] = q.front();
        q.pop();

        if (x == endX && y == endY) {
            vector<string> path;
            pair<int, int> cur = {endX, endY};
            while (!(cur.first == startX && cur.second == startY)) {
                path.push_back(move[cur]);
                cur = parent[cur];
            }
            return path;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i], ny = y + dy[i];

            if (isValid(nx, ny) && !visited[nx][ny]) {
                visited[nx][ny] = true;
                q.push({nx, ny});
                parent[{nx, ny}] = {x, y};
                move[{nx, ny}] = dir[i];
            }
        }
    }

    return {};
}

struct Berth
{
    /* 泊位 */
    int x,y;
    int time;
    int velocity;
    bool flag;
    int num; //计算当前泊位拥有的货物数量
    Berth () {
        flag = false;
    }
}berth[B_SIZE];

struct Boat
{
    /*
    shipedFrame 表示船在第几帧进入的港口
    num 表示 船当前装了多少货物
    */
    int shipedFrame, id, status, num, myLastBerth;
    void ship(int shipId, int berthId) {
        printf("ship %d %d\n", shipId, berthId);
    }

    void go(int shipID) const {
        printf("go %d\n", shipID);
    }
}boat[BOAT_SIZE];

struct Goods
{
    int x,y;
    int value;
    int frame;
    bool operator<(const Goods& other) const {
        return value < other.value;
    }
    Goods (int frame, int x, int y, int val) {
        this->frame = frame;
        this->x = x;
        this->y = y;
        this->value = val;
    }
};
priority_queue<Goods> goods_queue;

struct Robot
{
    int x,y;
    int mbx, mby; //?
    bool has_goods;
    int goods_value;
    int status; //0 free 1 work 2 collision
    int st;
    vector<string> directions;
    priority_queue<Goods> myGoods;
}robot[10];

/*
 * 优先队列用法
    while (!pq.empty()) {
        Item item = pq.top();
        pq.pop();
        std::cout << "Item value: " << item.value << std::endl;
    }
*/

/*Dijkstra*/
const int N = 200;
const int maxn = N * N;
const int INF = 0x3f3f3f3f;

vector<int> dist(maxn, INF);
 
const vector<pair<int, int>> directions{{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

void dijkstra(int start) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
    vector<bool> vis(maxn,false);

    fill(dist.begin(), dist.end(), INF);

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [cost, pos] = pq.top(); pq.pop();

        //cout << "now in point " << pos << endl;
        if (vis[pos])   continue;
        vis[pos] = 1;

        if (cost > dist[pos]) continue;

        int x = pos / N, y = pos % N;

        // 检查四个方向
        for (auto& dir : directions) {
            int newX = x + dir.first, newY = y + dir.second;
            if (newX >= 0 && newX < N && newY >= 0 && newY < N && mp[newX][newY] != '#' && mp[newX][newY] != '*') {
                int newIndex = newX * N + newY;
                int newCost = cost + 1;
                if (newCost < dist[newIndex]) {
                    dist[newIndex] = newCost;
                    pq.push({newCost, newIndex});
                }
            }
        }
    }
}

/*
    泊位分配：按照路径长度来为机器人分配初始泊位
    分配结果储存在choice数组中
    choice[i] = j 代表第i个机器人选择泊位j
    放在Init()中
*/
void robotBerthChoose() {
    vector<bool> has_chosen(10,false);    //泊位是否已经被选过
    int cnt = 0;
    vector<int> number4robot;  //如果到不了任何泊位
    for (int i = 0;i < 200; ++i) {
        if (cnt == 10)  break;
        for (int j  = 0;j < 200; ++j) {
            //初始阶段没有提供机器人坐标
            if (mp[i][j] == 'A') {
                //对每个机器人遍历10个berth位置
                fout << "*********** " << i << ' ' << j << endl;
                int minLength = 0x3f3f3f3f, minID = -1;
                for (int k = 0; k < 10; k++) {
                    dijkstra(i * 200 + j);
                    int l = dist[berth[i].x * 200 + berth[i].y];
                    if (l == INF)   continue;
                    if (l < minLength) {
                        minID = k;
                        minLength = l;
                    }
                }
                //单独处理一下四号机器人
                if (minLength == 0x3f3f3f3f) {
                    number4robot.push_back(cnt++);
                    continue;
                }
                choice[cnt++] = minID;
                has_chosen[minID] = 1;
            }
            if (cnt == 10)  break; 
        }
    }
    if (!number4robot.empty()) {
        for (int i = 0; i < 10; i++)
            if (has_chosen[i] == false) {
                choice[number4robot.back()] = i;
                number4robot.pop_back();
                has_chosen[i] = true;
            }
    }

    //for debug
    // for (int i = 0;i < 10; i++) {
    //     assert(has_chosen[i] == true);
    // }

}

void Init()
{
    for(int i = 0; i < M_SIZE; i ++)
        scanf("%s", mp[i]);
    for(int i = 0; i < B_SIZE; i ++)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].time, &berth[id].velocity);
        berthid[{berth[id].x,berth[id].y}] = id;    //berth坐标到id的映射
    }
    scanf("%d", &boat_capacity);
    
    fout << "这一轮的 船容量 为: " << boat_capacity << "\n";
    fout << "######################\n";
    
    char okk[100];
    scanf("%s", okk);
    robotBerthChoose();
    for (int i = 0;i < 10; ++i)
        fout << i << " chooses " << choice[i] << "berth.\n";
    printf("OK\n");
    fflush(stdout);
}

int Input()
{
    scanf("%d%d", &frame_id, &money);
    scanf("%d", &k);
    for(int i = 0; i < k; i ++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        Goods goods = {frame_id, x, y, val};
        goods_queue.push(goods);
    }
    for(int i = 0; i < 10; i ++)
    {
        scanf("%d%d%d%d", &robot[i].has_goods, &robot[i].x, &robot[i].y, &robot[i].st);
    }
    for(int i = 0; i < 5; i ++)
        scanf("%d%d\n", &boat[i].status, &boat[i].id);
    char okk[100];
    scanf("%s", okk);
    return frame_id;
}

bool boatLastChance (int frameID, int boatID) {
    if (boat[boatID].id != -1 && boat[boatID].status != 0) {
        return (15000 - frameID == berth[boat[boatID].id].time);
    } else if (boat[boatID].id != -1 && boat[boatID].status == 0) {
        return (15000 - frameID == berth[boat[boatID].myLastBerth].time);
    }
    return false;
}

// 找到拥有货物数量最多的泊位
int getMaxGoodsBerthID () {
    int maxGoods = 0;
    int BerthID = -1;
    for (int i = 0; i < 10; i++) {
        if (!berth[i].flag)
            if (berth[i].num >= maxGoods) {
                maxGoods = berth[i].num;
                BerthID = i;
            }
    }
    return BerthID;
}

bool isBoatFull (int boatID) {
    return boat[boatID].num == boat_capacity;
}

void boatAction (int frameID) {
    /* boat.status:
        0 : moving
        1 : finishing shipping OR waiting for loading
        2 : waiting for berth

        boat.id: id is the berth id; -1 means virtual point.
    */
    for (int i = 0; i < BOAT_SIZE; i ++) {
        if (boat[i].id == -1) {
            fout << "Boat " << i << " is going / at virtual point ! My Status is: " << boat[i].status << '\n';
        }
        if (isBoatFull(i)) {
            boat[i].go(i);
            berth[boat[i].myLastBerth].flag = false;
            boat[i].num = 0;
            continue;
        }
        
        // 判断是不是最后一次送货机会
        if (boatLastChance(frameID, i)) { //可能会导致时间上的开销增加
            boat[i].go(i);
            berth[boat[i].myLastBerth].flag = false;
            //取出港口中的货物
            fout << "!!!!!!!!!!!!Boat " << i << " now is the last chance : Berth time is " 
            << berth[boat[i].id].time << " Status is " << boat[i].status 
            << " Now the frame is " << frameID << "I have " << boat[i].num << "Goods\n";
            continue;
        }

        if (boat[i].status == 0)
            continue;
        if (boat[i].status == 2) {
            int berthID = getMaxGoodsBerthID();
            if (berthID == boat[i].id)
                boat[i].shipedFrame += 1;
            else {
                boat[i].ship(i, berthID);
                boat[i].shipedFrame = frameID + berth[berthID].time;
            }
        }
        if (boat[i].id == -1 && boat[i].status != 0) {
            int berthID = getMaxGoodsBerthID();
            fout << "$$$$$$$$$ Boat " << i << " chooses the " << berthID << endl;
            boat[i].myLastBerth = berthID;
            boat[i].ship(i, berthID);
            berth[berthID].flag = true;
            boat[i].shipedFrame = frameID + berth[berthID].time;
        } else if (boat[i].id != -1 && boat[i].status == 1) {
            boat[i].myLastBerth = boat[i].id;
            // 如果 此时泊位的货物数量小于泊位每帧装卸速度，那就直接讲 泊位num 赋值为0,否则就让让 nun - 装卸速度 velocity
            // 如果 船当前剩余容量小于泊位装卸速度 那么这一帧 最多装我还差的量
            if (berth[boat[i].id].num < berth[boat[i].id].velocity) {
                boat[i].num += berth[boat[i].id].num;
                berth[boat[i].id].num = 0;
            } else if (boat_capacity - boat[i].num < berth[boat[i].id].velocity) {
                boat[i].num = boat_capacity;
                berth[boat[i].id].num -= (boat_capacity - boat[i].num);
            } else {
                boat[i].num += berth[boat[i].id].velocity;
                berth[boat[i].id].num -= berth[boat[i].id].velocity;
            }
            
            // 如果我把这个港口装空了 我就找下一个
            if (!isBoatFull(i) && berth[boat[i].id].num == 0) {
                berth[boat[i].myLastBerth].flag = false;
                int berthID = getMaxGoodsBerthID();
                fout << "$$$$$$$$$ Boat " << i << " chooses the " << berthID << endl;
                boat[i].ship(i, berthID);
                berth[berthID].flag = true;
                boat[i].shipedFrame = frameID + 500;
            }
        }
        fout << "Boat: " << i << " Status is " << boat[i].status << " Now is at " << boat[i].id <<"\n";
    }
}

void robot_move(int robot_id){
    if (!robot[robot_id].directions.empty()){
        const char* direc = robot[robot_id].directions.back().c_str();
        printf("move %d %s\n", robot_id,direc);
        robot[robot_id].directions.pop_back();
    } else {
        if (robot[robot_id].x != robot[robot_id].mbx || robot[robot_id].y != robot[robot_id].mby) {
            robot[robot_id].status = 0;
            return;
        }
    }
    return;
}

void getGoods(int robot_id){
    //[robot_id].has_goods = true;
    //修改goods状态
    //goods[goods_id].value = 0;
    printf("get %d\n", robot_id);
}

void pullGoods(int robot_id){
    robot[robot_id].status = 0;

    //增加泊位的货物
    int bid = berthid[{robot[robot_id].mbx,robot[robot_id].mby}];
    berth[bid].num += 1;
    fout << "######### " << robot_id << " pull a goods in (" << robot[robot_id].x << "," <<robot[robot_id].y << ") its berth is (" << robot[robot_id].mbx << "," <<robot[robot_id].mby << ")\n";

    printf("pull %d\n", robot_id);
}

void changeDirection(int rid, int frame) {
    string tmp = robot[rid].directions.back();
    
    //如果剩下三个个方向全是墙就不改变方向
    // int i;
    // for (i = 0;i < 4; ++i) {
    //     if (dir[i] == tmp)  continue;
    //     if (mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '#' && mp[robot[rid].x + dx[i]][robot[rid].y + dy[i]] != '*')
    //         break;
    // }
    // if (i == 4) return;

    // if (frame % 20 == 0) {
        robot[rid].mbx = berth[rand() % 10].x;
        robot[rid].mby = berth[rand() % 10].y;
    // }
    
    //随机选择一个可以走的位置
    // int c,x,y,newx,newy;
    // do {
    //     c = rand() % 4;
    //     x = dx[c], y = dy[c];
    //     newx = robot[rid].x + x, newy = robot[rid].y + y;
    // } while (dir[c] == tmp || mp[newx][newy] == '#' || mp[newx][newy] == '*');

    int newx = robot[rid].x;
    int newy = robot[rid].y;

    //更新路径
    vector<string> t = BFS(newx,newy,robot[rid].mbx,robot[rid].mby);
    if (!t.empty()) {
        //如果碰巧路径不可达那就等下次碰撞了再选一次，这里不处理
        robot[rid].directions = t;
    // if (!t.empty())
    //     robot[rid].directions.push_back(dir[c]);
    }
}

int main() {
    Init();
    srand((unsigned)time(NULL));
    for(int frame = 1; frame <=15000; frame ++){
        int frame_id = Input();
        //第一帧以及每500帧操作一下船
        // if (frame == 1 || frame % 500 == 0)
        // 船的操作， 传入当前帧ID
        if (frame >= 700)
            boatAction(frame_id);

        //新增的货物入队 集合到 Input 函数中了
        
        for (int i = 0; i < 10; ++i) {   
            // if (i == 4) // 对map1 的特殊处理
            //     continue;      
            if (robot[i].status == 0 && robot[i].st != 0 && robot[i].has_goods) {
                robot[i].status = 1;
            }   
            priority_queue<Goods> temp;
            //找到空闲的机器人就去分配货物
            while (!goods_queue.empty() && robot[i].status == 0 && robot[i].st != 0 && !robot[i].has_goods) {
                Goods tmp = goods_queue.top();
                if (-50 <= tmp.x - robot[i].x && tmp.x - robot[i].x <= 50 && -50 <= tmp.y - robot[i].y && tmp.y - robot[i].y <= 50) {
                    robot[i].myGoods.push(tmp);
                    goods_queue.pop();
                } else {
                    temp.push(goods_queue.top());
                    goods_queue.pop();
                }
            }
            while (!temp.empty()) {
                goods_queue.push(temp.top());
                temp.pop();
            }
            if (!robot[i].myGoods.empty()) {
                while (frame_id - robot[i].myGoods.top().frame >= 1000) {
                    robot[i].myGoods.pop();
                }
                Goods tmp = robot[i].myGoods.top();
                robot[i].myGoods.pop();
                robot[i].directions = BFS(robot[i].x,robot[i].y,tmp.x,tmp.y);
                robot[i].mbx = tmp.x;
                robot[i].mby = tmp.y;
                robot[i].goods_value = tmp.value;
                robot[i].status = 1;
            }
            

            //如果没带货物碰撞
            if (robot[i].st == 0 && !robot[i].has_goods) {
                robot[i].status = 0;
                robot[i].directions = {};
                robot[i].mbx = robot[i].mby = 0;
                continue;
            }

            //带着货物碰撞
            if (robot[i].st == 0 && robot[i].has_goods) {
                //changeDirection(i, frame_id);
                has_cracked[i] = 1;
                continue;
            }

            if (robot[i].status == 1 && !robot[i].has_goods) {
                //在拿货物的路上
                if (robot[i].x == robot[i].mbx && robot[i].y == robot[i].mby) {
                    robot[i].status = 0;
                    robot[i].mbx = robot[i].mby = 0;
                }
                
                robot_move(i);
                if (robot[i].directions.empty()) {
                    //去拿货物
                    getGoods(i);
                } 
            } else if (robot[i].status == 1 && robot[i].has_goods) {
                //拿到货物去泊位
                // int minLen = 1000000;
                // vector<string> tmp;
                // //泊位选择
                // for(int j = 0; j < 10; j ++){
                //     vector<string> one = BFS(robot[i].x, robot[i].y,berth[j].x,berth[j].y);
                //     if(!one.empty() && one.size() < minLen){
                //         minLen = one.size();
                        
                //         tmp = one;
                //     }
                // }
                if (has_cracked[i] == 1) {
                    changeDirection(i, frame_id);
                    has_cracked[i] = 0;
                }
                
                if (robot[i].directions.empty()) {
                    vector<string> tmp = BFS(robot[i].x, robot[i].y,berth[choice[i]].x,berth[choice[i]].y);
                    if (!tmp.empty()) {
                        robot[i].directions = tmp;
                        robot[i].mbx = berth[i].x;
                        robot[i].mby = berth[i].y;
                    }
                }
            
                robot_move(i);
                if (robot[i].directions.empty()) {
                    pullGoods(i);
                }
            }
        }
        while (!goods_queue.empty()) {
            goods_queue.pop();
        }
        puts("OK");
        //输出每一帧的泊位剩余货物大小
        fout << "frame_id: " << frame_id << endl;
        fout << "----------" << endl;
        for (int i = 0;i < 10; i++) {
            fout << "Berth " << i << ": " << berth[i].num << endl;
        }
        fout << "----------" << endl;
        fflush(stdout);
    }
    return 0;
}