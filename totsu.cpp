#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <time.h>
using namespace std;

struct City
{
    int id;
    double x, y;
};

struct Load
{
    City c1, c2;
};

// 符号付面積を計算する関数
double area2(City a, City b, City c)
{
    return ((b.x - a.x) * (c.y - a.y)) - ((c.x - a.x) * (b.y - a.y));
}

int distance(City c1, City c2)
{
    return sqrt(((c1.x - c2.x) * (c1.x - c2.x)) + ((c1.y - c2.y) * (c1.y - c2.y)));
}

int distanceLoadAndNewCity(Load l, City c)
{
    return distance(l.c1, c) + distance(l.c2, c) - distance(l.c1, l.c2);
}

// 偏角を計算する関数
double calculateAngle(const City &reference, const City &target)
{
    return atan2(target.y - reference.y, target.x - reference.x);
}

// ２点のPointを入れ替える関数
void swap_city(vector<City> &cities, int i, int j)
{
    City tmp = cities[i];
    cities[i] = cities[j];
    cities[j] = tmp;
}

void merge(vector<City> &cities, int left, int mid, int right, City zero)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<City> L(n1), R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = cities[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = cities[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
    {
        // if (calculateAngle(zero,L[i]) <= calculateAngle(zero,R[j])) {
        if (area2(zero, L[i], R[j]) > 0)
        {
            cities[k] = L[i];
            i++;
        }
        else
        {
            cities[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        cities[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        cities[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(vector<City> &cities, int left, int right, City zero)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
        mergeSort(cities, left, mid, zero);
        mergeSort(cities, mid + 1, right, zero);
        merge(cities, left, mid, right, zero);
    }
}

// x座標に基づいてCityオブジェクトを比較するカスタム比較関数
bool compareByX(const City &a, const City &b)
{
    return a.x < b.x;
}

int main(int argc, char *argv[])
{
    clock_t start_time, end_time;
    double cpu_time;
    start_time = clock();

    if (argc < 2)
    {
        cerr << "使用法: " << argv[0] << " <ファイル名>" << endl;
        return 1;
    }

    string filename = argv[1];
    ifstream file(filename);
    if (!file)
    {
        cerr << "ファイルを開けません: " << filename << endl;
        return 1;
    }

    string line;
    int n = 0;
    vector<City> cities;

    while (getline(file, line))
    {
        // string::npos は検索が失敗したことを示す値
        if (line.find("DIMENSION") != string::npos)
        {
            // sscanfは文字列(char型配列)から入力受け取る
            // c_str()cスタイル(char型配列)文字列を返す
            sscanf(line.c_str(), "DIMENSION : %d", &n);
        }
        if (line == "NODE_COORD_SECTION")
        {
            int id;
            double x, y;
            for (int i = 0; i < n; ++i)
            {
                file >> id >> x >> y;
                cities.push_back({id, x, y});
            }
        }
    }

    int dis[n][n];
    for (int i = 0; i < n; ++i)
    {
        for (int j = i; j < n; ++j)
        {
            if (i != j)
            {
                dis[i][j] = distance(cities[i], cities[j]);
                dis[j][i] = dis[i][j];
            }
            else
            {
                dis[i][j] = 0;
            }
        }
    }
    //O(n^2)

    vector<City> cities_c = cities;

    for (int i = 1; i < n; ++i)
    { // x座標が最小の点を先頭にする
        if (cities[i].x < cities[0].x)
        { // 例外処理（最小のx座標２つあるとき）
            swap_city(cities, i, 0);
        }
    }

    mergeSort(cities, 0, cities.size() - 1, cities[0]);
    // ここまでで偏角順にソート完了

    City v[n];
    v[0] = cities[0];
    v[1] = cities[1];
    v[2] = cities[2];
    int k = 2;
    for (int i = 3; i < n; ++i)
    {
        while (area2(v[k - 1], v[k], cities[i]) <= 0)
        { // 例外処理（同一直線上に３点以上ある場合）
            --k;
            if (k < 1)
                break;
        }
        ++k;
        v[k] = cities[i];
    }

    vector<Load> routes;
    int check[n];
    for (int i = 0; i < n; ++i)
    {
        check[i] = 0;
    }
    for (int i = 0; i < k; ++i)
    {
        routes.push_back({v[i], v[i + 1]});
        check[v[i].id - 1] = 1;
    }
    routes.push_back({v[k], v[0]});
    check[v[k].id - 1] = 1;

    std::sort(cities_c.begin(), cities_c.end(), compareByX);
    // 挿入法スタート
    for (int i = 0; i < n; ++i)
    {
        if (check[cities_c[i].id - 1] == 0)
        {
            Load min_l;
            int cnt = 0;
            int min_it = 0;
            for (const auto &route : routes)
            {
                if (cnt == 0)
                {
                    min_l = route;
                }
                else if (distanceLoadAndNewCity(route, cities_c[i]) < distanceLoadAndNewCity(min_l, cities_c[i]))
                {
                    min_l = route;
                    min_it = cnt;
                }
                cnt++;
            }
            routes.push_back({min_l.c1, cities_c[i]});
            routes.push_back({cities_c[i], min_l.c2});
            routes.erase(routes.begin() + min_it);
            check[cities_c[i].id - 1] = 1;
        }
    }

    int sum = 0;
    for (const auto &route : routes)
    {
        sum += distance(route.c1, route.c2);
    }
    
    cout << "解: " << sum << "\n";

    end_time = clock();
    cpu_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    // 計算時間を出力する場合は以下の行を追加
    printf("計算時間:%.30f秒\n", cpu_time);

    FILE *gp = popen("gnuplot -persist", "w");
    if (!gp)
    {
        cerr << "gnuplotを開けませんでした。" << endl;
        return 1;
    }

    // gnuplotに設定を送信
    fprintf(gp, "set title 'Routes'\n");
    fprintf(gp, "set xlabel 'X'\n");
    fprintf(gp, "set ylabel 'Y'\n");
    fprintf(gp, "set grid\n");
    fprintf(gp, "plot '-' with points,'-' with lines\n");

    // routesの各要素に対して点をプロット
    for (const auto &route : routes)
    {
        fprintf(gp, "%f %f\n", route.c1.x, route.c1.y);
    }
    fprintf(gp, "e\n"); // データの終了を示す

    // routesの各要素に対して辺をプロット
    for (const auto &route : routes)
    {
        fprintf(gp, "%f %f\n", route.c1.x, route.c1.y);
        fprintf(gp, "%f %f\n", route.c2.x, route.c2.y);
        fprintf(gp, "\n"); // 辺の終了
    }
    fprintf(gp, "e\n"); // データの終了を示す

    // gnuplotを閉じる
    pclose(gp);

    return 0;
}