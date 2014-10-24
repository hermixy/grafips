#pragma once
#include <map>
#include <vector>
#include <GLES2/gl2.h>

// interpolates data, provides it to view
class DataPoint;

class GraphSet
{
  public:
    struct Point {
        GLfloat x;
        GLfloat y;
    };
    typedef std::vector<Point> PointVec;

    GraphSet();
    void Add(const DataPoint &d);
    void SetLimit(int max_data_age);
    void GetData(PointVec *data);

  private:
    std::map<int, float> m_data;
    int m_current_time;
    int m_max_data_age;
};

