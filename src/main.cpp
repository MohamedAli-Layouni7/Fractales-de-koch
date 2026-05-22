#include <chrono>
#include <cstring>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <string>
#include <type_traits>
#include <vector>
using namespace std;
typedef unsigned long index_;
typedef unsigned long size_;
typedef size_ point_index_;
typedef size_ array_inc;
typedef unsigned char byte_;
template <typename T> struct point {
  T x, y;
};
typedef double angle_type;
struct rotator {
  angle_type angle;
  double ca, sa;
  void set_angle(angle_type angle) {
    this->angle = angle;
    this->ca = cos(angle);
    this->sa = sin(angle);
  }
  template <typename T> inline point<T> rotate(point<T> &a) {
    return {this->ca * a.x - this->sa * a.y, this->sa * a.x + this->ca * a.y};
  }
  template <typename T> inline point<T> rotate(point<T> &&a) {
    return {this->ca * a.x - this->sa * a.y, this->sa * a.x + this->ca * a.y};
  }
};
template <typename T> point<T> operator+(const point<T> &a, const point<T> &b) {
  return {a.x + b.x, a.y + b.y};
}
template <typename T> point<T> operator-(const point<T> &a, const point<T> &b) {
  return {a.x - b.x, a.y - b.y};
}
template <typename T> point<T> operator*(T a, const point<T> &b) {
  return {a * b.x, a * b.y};
}
template <typename T> point<T> operator*(const point<T> &b, T a) {
  return a * b;
}
template <typename T> ostream &operator<<(ostream &os, point<T> &b) {
  os << "{ " << setw(12) << setprecision(4) << b.x << " , " << setw(12)
     << setprecision(4) << b.y << " }" << endl;
  return os;
}
long fast_power(long base, long exponent) {
  if (exponent < 0)
    return 0;
  long result = 1;
  while (exponent) {
    if (exponent & 1) {
      result *= base;
    }
    base *= base;
    exponent = exponent >> 1;
  }
  return result;
}
class koch_fractal {
private:
  byte_ initial_segment_number, new_element_order, fractal_order;
  size_ segment_number;
  size_ point_number;
  double initial_fractal_raduis, central_angle, factor;
  vector<point<double>> points;
  rotator rot;
  void update_segment_number() {
    this->segment_number =
        this->initial_segment_number *
        fast_power(this->new_element_order + 1, this->fractal_order);
  }
  void create_initial_curve() {
    if (this->initial_segment_number == 1) {
      this->points[0] = {-this->initial_fractal_raduis, 0};
      this->points[this->segment_number] = {this->initial_fractal_raduis, 0};
      return;
    }
    const size_ inc = this->segment_number / this->initial_segment_number;
    const double angle_factor = 2 * M_PI / this->initial_segment_number;
    for (int i = 0; i <= this->initial_segment_number; i++) {
      this->points[inc * i] = {
          this->initial_fractal_raduis * cos(angle_factor * i),
          this->initial_fractal_raduis * sin(angle_factor * i)};
    }
  }
  inline void fractalize_segment(point_index_ begin, point_index_ end,
                                 array_inc inc) {
    point<double> A = this->points[begin], B = this->points[end];
    begin += inc;
    end -= inc;
    this->points[begin] = (A + (B - A) * this->factor);
    this->points[end] = (B + (A - B) * this->factor);
    this->points[begin + inc] =
        (this->points[begin] +
         this->rot.rotate(this->points[end] - this->points[begin]));
    begin += inc;
    for (point_index_ i = begin; i < end; i += inc) {
      this->points[i + inc] =
          (this->points[i] +
           this->rot.rotate(this->points[i - inc] - this->points[i]));
    }
  }

public:
  koch_fractal(byte_ isn, byte_ neo, byte_ fo, double raduis, double factor) {
    this->initial_segment_number = isn;
    this->new_element_order = neo;
    this->fractal_order = fo;
    this->factor = factor;
    this->update_segment_number();
    this->point_number = this->segment_number + 1;
    this->initial_fractal_raduis = raduis;
    this->central_angle = 2 * M_PI / this->new_element_order;
    this->rot.set_angle(M_PI + this->central_angle);
    this->points = vector<point<double>>(this->point_number);
  }
  void compute() {
    this->create_initial_curve();
    size_ limit = this->initial_segment_number;
    for (int i = 0; i < this->fractal_order; i++) {
      size_ inc = this->segment_number / limit;
// #pragma omp parallel for
      for (int j = 0; j < this->segment_number; j += inc) {
        this->fractalize_segment(j, j + inc,
                                 inc / (this->new_element_order + 1));
      }
      limit *= (this->new_element_order + 1);
    }
  }
  friend ostream &operator<<(ostream &os, koch_fractal &frac);
  void generate_svg(fstream &file_object) {
    char line[] = "<line x1=\"%09.4lf\" y1=\"%09.4lf\" x2=\"%09.4lf\" "
                  "y2=\"%09.4lf\" stroke=\"blue\" stroke-width=\"0.25\" />",
         svg[] =
             "<svg width=\"500\" height=\"500\" viewBox=\"-250 -100 500 200\" "
             "xmlns=\"http://www.w3.org/2000/svg\" "
             "xmlns:xlink=\"http://www.w3.org/1999/xlink\"></svg>";
    int line_final_size = sizeof(line) + 7, svg_size = sizeof(svg),
        header_size = svg_size - 7;
    unsigned long total_size =
        this->segment_number * (line_final_size) + svg_size;
    cout << total_size << endl;
    char *ret = static_cast<char *>(malloc(total_size));
    if (ret == nullptr)
      return;
    strncpy(ret, svg, header_size);
    char *p = ret + header_size;
    for (point_index_ i = 0; i < this->segment_number; i++) {
      sprintf(p, line, this->points[i].x, this->points[i].y,
              this->points[i + 1].x, this->points[i + 1].y);
      p += line_final_size;
    }
    strncpy(p, svg + header_size, 6);
    ret[total_size - 1] = 0;
    file_object << ret;
    free(ret);
  }
};

ostream &operator<<(ostream &os, koch_fractal &frac) {
  for (point_index_ i = 0; i < frac.point_number; i++) {
    os << frac.points[i];
  }
  return os;
}

double mean(vector<int> v) {
  double m = 0;
  for (int i = 0; i < v.size(); i++) {
    m += v[i];
  }
  return m / v.size();
}

double stddev(vector<int> v, double mean) {
  double m = 0;
  for (int i = 0; i < v.size(); i++) {
    double temp = v[i] - mean;
    m += (temp * temp);
  }
  return sqrt(m / v.size());
}

int main() {
  koch_fractal frac(2, 15, 5, 100.0, 1.0 / 3);
  constexpr unsigned int limit = 1000;
  vector<int> times(limit);
  for (int i = 0; i < limit; i++) {
    auto start = chrono::system_clock::now();
    frac.compute();
    auto end = chrono::system_clock::now();
    times[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                   .count();
  }
  double m = mean(times);
  cout << "Le temps d'exécution moyen:\t" << m / fast_power(10, 6)
       << " millisecondes" << endl;
  cout << "Le variation standard du temps d'exécution:\t"
       << stddev(times, m) / fast_power(10, 6) << " millisecondes" << endl;
  // cout << frac;
  fstream file("out.svg", ios_base::openmode::_S_out);
  frac.generate_svg(file);
  return 0;
}
