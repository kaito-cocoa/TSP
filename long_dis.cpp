// 距離長い3道スタート
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

int distance(City c1, City c2)
{
    return sqrt(((c1.x - c2.x) * (c1.x - c2.x)) + ((c1.y - c2.y) * (c1.y - c2.y)));
}

int distanceLoadAndNewCity(Load l, City c)
{
    return distance(l.c1, c) + distance(l.c2, c);
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

    std::vector<std::vector<int>> dis(n, std::vector<int>(n));
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

    int l1 = 0, l2 = 1, l3 = 2;
    double max_sum = dis[0][1] + dis[1][2] + dis[0][2];
    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            for (int k = j + 1; k < n; ++k)
            {
                if ((dis[i][j] + dis[j][k] + dis[i][k]) > max_sum)
                {
                    l1 = i;
                    l2 = j;
                    l3 = k;
                    // cout << max_sum << " → " << dis[l1][l2] + dis[l2][l3] + dis[l1][l3] << "\n";
                    max_sum = dis[l1][l2] + dis[l2][l3] + dis[l1][l3];
                }
            }
        }
    }

    cout << "ok1\n";

    vector<Load> routes;
    routes.push_back({cities[l1], cities[l2]});
    routes.push_back({cities[l2], cities[l3]});
    routes.push_back({cities[l1], cities[l3]});

    for (int i = 0; i < n; ++i)
    {
        if ((i == l1) || (i == l2) || (i == l3))
        {
            continue;
        }
        double min = distanceLoadAndNewCity(routes[0], cities[i]);
        int min_r_it = 0;
        for (int j = 0; j < routes.size() - 1; ++j)
        {
            if (distanceLoadAndNewCity(routes[j], cities[i]) < min)
            {
                min = distanceLoadAndNewCity(routes[j], cities[i]);
                min_r_it = j;
            }
        }
        routes.push_back({routes[min_r_it].c1, cities[i]});
        routes.push_back({cities[i], routes[min_r_it].c2});
        routes.erase(routes.begin() + min_r_it);
    }

    cout << "ok2\n";

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
    fprintf(gp, "plot '-' with points title 'Cities', '-' with lines title 'Routes'\n");

    // 全ての都市に対して点をプロット
    for (const auto &city : cities)
    {
        fprintf(gp, "%f %f\n", city.x, city.y);
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