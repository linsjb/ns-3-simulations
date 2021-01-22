#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "string"

#include <cmath>
#include <math.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Project");

class File_writer {
private:
  std::string filename;
  std::vector<std::vector<double>> data;

public:
  void set_filename(std::string filename) { this->filename = filename; }

  void append(std::vector<double> data) { this->data.push_back(data); }

  void write() {
    std::fstream output;
    output.open(this->filename + ".csv", std::fstream::out);

    if (output.is_open()) {
      if (this->data.size() > 0) {
        for (unsigned int row = 0; row < this->data.size(); row++) {
          for (unsigned int col = 0; col < this->data[row].size(); col++) {
            output << this->data[row][col];

            if (col != this->data[row].size() - 1) {
              output << ", ";
            }
          }

          output << "\n";
        }
      }
    }
  }
};

std::vector<double> lcg_rand(long a, int c, int m, int seed, int n) {
  std::vector<double> values;

  for (int i = 0; i < n; i++) {
    double max = 1.0 / (1.0 + (m - 1));
    seed = (int)(a * seed + c) % (int)m;

    if (seed < 0)
      seed += m;

    values.push_back((double)seed * max);
  }

  return values;
}

std::vector<double> poisson(std::vector<double> U, double lambda, int length) {
  for (int i = 0; i < length; i++) {
    U[i] = -((std::log(U[i])) / lambda);
  }

  return U;
}

std::vector<double> ns3_urv(double min, double max, int n) {
  std::vector<double> data;

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
  x->SetAttribute("Min", DoubleValue(min));
  x->SetAttribute("Max", DoubleValue(max));

  for (int i = 0; i < n; i++) {
    data.push_back(x->GetValue());
  }

  return data;
}

std::vector<double> ns3_rvn(double mean, double bound, int n) {
  std::vector<double> data;

  Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable>();
  x->SetAttribute("Mean", DoubleValue(mean));
  x->SetAttribute("Bound", DoubleValue(bound));

  for (int i = 0; i < n; i++) {
    data.push_back(x->GetValue());
  }

  return data;
}

std::vector<double> prob_func(int n) {
  std::vector<double> data;
  int pdf = 0;
  for (int i = 0; i < n; i++) {
    pdf = rand() % 200;
    if (pdf > 360)
      std::cout << 1 << " ";
    else if (pdf < 0)
      std::cout << 0 << " ";
    else
      data.push_back(pdf * 0.1 / 360);
  }

  return data;
}

int main(int argc, char *argv[]) {
  double min = 0.0, max = 1.0, lambda = 3.14, bound = 1.0;
  int a = 13, c = 1, m = 100, seed = 1, n = 1000;
  bool lcg = false, ns3 = false, all = false, poi = false, rvn = false,
       pdf = false, write_to_file = true;

  CommandLine cmd;
  cmd.AddValue("min", "", min);
  cmd.AddValue("max", "", max);
  cmd.AddValue("lambda", "", lambda);
  cmd.AddValue("bound", "", bound);
  cmd.AddValue("m", "", m);
  cmd.AddValue("a", "", a);
  cmd.AddValue("c", "", c);
  cmd.AddValue("seed", "", seed);
  cmd.AddValue("n", "", n);

  cmd.AddValue("lcg", "", lcg);
  cmd.AddValue("ns3", "", ns3);
  cmd.AddValue("poi", "", poi);
  cmd.AddValue("rvn", "", rvn);
  cmd.AddValue("pdf", "", pdf);
  cmd.AddValue("all", "", all);

  cmd.Parse(argc, argv);

  File_writer fw;
  // fw.set_filename("m" + std::to_string(m) + "-c" + std::to_string(c) + "-a" +
  // std::to_string(a));
  fw.set_filename("poirvn-m" + std::to_string(m) + "-c" + std::to_string(c) +
                  "-a" + std::to_string(a));

  if (all)
    lcg = ns3 = rvn = poi = pdf = true;

  if (lcg)
    fw.append(lcg_rand(a, c, m, seed, n));

  if (ns3)
    fw.append(ns3_urv(min, max, n));

  if (poi)
    fw.append(poisson(lcg_rand(a, c, m, seed, n), lambda, n));

  if (rvn)
    fw.append(ns3_rvn(lambda, bound, n));
  if (pdf)
    fw.append(prob_func(n));

  if (write_to_file)
    fw.write();

  return 0;
}
