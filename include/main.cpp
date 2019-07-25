#define FMT_HEADER_ONLY

#include "csv.hpp"
#include "cxxopts.hpp"
#include "fmt/format.h"
#include "random_t/random_t.hpp"
#include "tupletools.hpp"
#include "utils.cpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

constexpr int MAX_2015 = 11'810'384;
constexpr int MAX_2019 = 15'034'520;

struct erate_t
{
  std::string lea_number;
  int discount, cost, no_mutate, bucket;
};

class Critter
{
public:
  std::vector<std::tuple<int, int>> _genes;
  double _fitness;
  bool _skip;

  Critter(int N)
    : _fitness(0)
    , _skip(false)
  {
    _genes.resize(N);
    std::fill(begin(_genes), end(_genes), std::forward_as_tuple(0, 0));
  }

  Critter(int N, std::vector<std::tuple<int, int>>&& g)
    : _fitness(0)
    , _skip(false)
  {
    _genes.resize(N);
    this->genes(g);
  }

  auto genes() -> std::vector<std::tuple<int, int>>& { return _genes; }
  auto genes() const -> const std::vector<std::tuple<int, int>>
  {
    return _genes;
  }
  void genes(std::vector<std::tuple<int, int>> const& g)
  {
    std::copy(begin(g), end(g), begin(_genes));
  }

  double fitness() { return _fitness; }
  double fitness() const { return _fitness; }
  void fitness(double f) { _fitness = f; }

  bool skip() { return _skip; }
  bool skip() const { return _skip; }
  void skip(bool s) { _skip = s; }

  friend std::ostream& operator<<(std::ostream& os, Critter const& data)
  {
    fmt::memory_buffer c;
    fmt::format_to(c,
                   "{{\nfitness: {0},\nskip: {1},\ngenes:\n\t[",
                   data.fitness(),
                   data.skip());
    for (auto& [g1, g2] : data.genes()) {
      fmt::format_to(c, "\n\t\t[{0}, {1}]", g1, g2);
    }
    fmt::format_to(c, "\n\t]\n}}");
    os << c.data();
    return os;
  }
};

auto
parent_distb(int N) -> std::vector<int>
{

  int slice = ceil(100.0 / N);
  if (N * slice > 100) {
    std::vector<int> t(N, slice);
    t[N - 1] = (100 - slice * (N - 1));
    return t;
  } else {
    std::vector<int> t(N, slice);
    return t;
  }
}

auto
make_genes(std::vector<erate_t>& data, int bucket_count)
  -> std::vector<std::tuple<int, int>>
{
  std::vector<std::tuple<int, int>> genes;
  genes.reserve(data.size());

  for (auto d : data) {
    std::tuple<int, int> tup;
    if (d.no_mutate) {
      tup = std::make_tuple(d.no_mutate, d.bucket);
    } else {
      tup = std::make_tuple(0, 0);
    }
    genes.push_back(tup);
  }
  return genes;
}

template<typename T>
void
calc_erate_stats(std::vector<erate_t>& data,
                 std::map<int, std::map<std::string, double>>& buckets,
                 Critter& critter,
                 int max_bucket,
                 T& rng)
{
  for (int i = 0; i < data.size(); i++) {
    int j = std::get<0>(critter.genes()[i]) ? std::get<1>(critter.genes()[i])
                                            : rng.randrange(0, buckets.size());
    while (buckets[j]["count"] >= max_bucket) {
      j = rng.randrange(0, buckets.size());
    }
    auto& bucket = buckets[j];

    bucket["total_cost"] += data[i].cost;
    bucket["total_discount"] += data[i].discount;
    bucket["count"]++;

    std::get<1>(critter.genes()[i]) = j;
  }

  double fitness = 0;
  for (auto& [_, bucket] : buckets) {
    if (bucket["count"] > 0) {
      bucket["average_discount"] =
        double_round((bucket["total_discount"] / (bucket["count"] * 100)), 3);
      bucket["discount_cost"] =
        bucket["average_discount"] * bucket["total_cost"];
      fitness += bucket["discount_cost"];
    }
    for (auto& [key, value] : bucket) { value = 0; }
  }
  critter.fitness(fitness);
  //   critter.skip(true);
}

template<typename T>
Critter
mate_critters(std::vector<Critter>& parents,
              std::vector<int>& pdistb,
              int N,
              T& rng)
{
  Critter child(N);

  std::sort(begin(parents), end(parents), [](auto c1, auto c2) {
    return c1.fitness() > c2.fitness();
  });

  int p = 0;
  for (int i = 0; i < child.genes().size(); i++) {
    int r = rng.randrange(0, 100);
    int start = 0;

    for (int j = 0; j < pdistb.size(); j++) {
      if (start < r && r < (start + pdistb[j])) {
        p = j;
        break;
      } else {
        start += pdistb[j];
      }
    }
    std::get<0>(child.genes()[i]) = std::get<0>(parents[p].genes()[i]);
    std::get<1>(child.genes()[i]) = std::get<1>(parents[p].genes()[i]);
  }
  return child;
}

template<typename T>
void
cull_mating_pool(std::vector<erate_t>& data,
                 std::vector<int>& pdistb,
                 std::vector<Critter>& critters,
                 int bucket_count,
                 int max_fitness,
                 int mutation_count,
                 int mating_pool_count,
                 int top_pool,
                 T& rng)
{
  int N = data.size();
  int M = critters.size();

  std::sort(begin(critters), end(critters), [](auto c1, auto c2) {
    return c1.fitness() > c2.fitness();
  });

  std::vector<Critter> children(begin(critters), begin(critters) + top_pool);
  std::vector<Critter> parents(begin(critters),
                               begin(critters) + mating_pool_count);
  children.reserve(N);

  for (int i = 0; i < M - top_pool; i++) {
    for (int j = 0; j < mating_pool_count; j++) {
      int r = rng.randrange(0, M);
      parents[j] = critters[r];
    }

    Critter child = mate_critters(parents, pdistb, N, rng);

    for (int j = 0; j < mutation_count; j++) {
      int r = rng.randrange(0, N);
      std::get<0>(child.genes()[r]) ^= 1;
    }
    children.push_back(child);
  }
  critters = std::move(children);
}

void
optimize_buckets(std::vector<erate_t>& data,
                 std::string out_file,
                 int bucket_count,
                 int max_bucket,
                 int population_count,
                 int mutation_rate,
                 int mutation_threshold_low,
                 int mutation_threshold_high,
                 int parent_count,
                 int mating_pool_count,
                 int iterations,
                 uint64_t rng_state)
{
  std::ofstream ofs;

  int N = data.size();
  int top_pool = population_count / 10;
  int mutation_count =
    std::min(static_cast<int>((N * mutation_rate) / 100.0f), N);
  mating_pool_count = std::min(mating_pool_count, N);
  mutation_threshold_high = std::max(
    static_cast<int>(1.0f * mutation_threshold_high / mutation_threshold_low),
    1);
  max_bucket = max_bucket > N ? N : max_bucket;

  std::vector<int> pdistb = parent_distb(parent_count);
  std::map<int, std::map<std::string, double>> buckets;

  random_t::Random rng(rng_state, random_t::lcg_xor_rot);

  double max_fitness = 0;
  for (int i = 0; i < bucket_count; i++) {
    buckets[i] = std::map<std::string, double>{{"average_discount", 0},
                                               {"total_discount", 0},
                                               {"total_cost", 0},
                                               {"discount_cost", 0},
                                               {"count", 0}};
  }
  std::vector<Critter> critters(population_count,
                                {N, make_genes(data, bucket_count)});

  int mutation_counter_low = 0;
  int mutation_counter_high = 0;
  int mutation_gap = 0;
  int t_mutation_count = mutation_count;

  for (int i = 0; i < iterations; i++) {
    for (auto& critter : critters) {
      if (!critter.skip()) {
        calc_erate_stats(data, buckets, critter, max_bucket, rng);

        if (critter.fitness() > max_fitness) {
          ofs.open(out_file, std::ios::trunc);

          std::string header =
            fmt::format("\niteration: {0}, discount-total: {1:.2f}\n"
                        "max-delta: {2:.2f}, prev-max-delta: {3:.2f}\n"
                        "iteration-delta: {4}, rng-state: {5}",
                        i,
                        critter.fitness(),
                        critter.fitness() - MAX_2019,
                        critter.fitness() - max_fitness,
                        i - mutation_gap,
                        rng.state());

          max_fitness = critter.fitness();
          mutation_counter_low = i;
          mutation_counter_high = 0;
          mutation_gap = i;

          fmt::print("{0}\n", header);
          ofs << "lea-number,discount,cost,no-mutate,bucket\n";

          fmt::memory_buffer row;
          for (int k = 0; k < N; k++) {
            fmt::format_to(row,
                           "{0},{1},{2},{3},{4}\n",
                           data[k].lea_number,
                           data[k].discount,
                           data[k].cost,
                           std::get<0>(critter.genes()[k]),
                           std::get<1>(critter.genes()[k]));
            std::get<0>(critter.genes()[k]) = 1;
          }
          ofs << row.data();
          ofs.close();
        }
      }
    }
    if ((i - mutation_counter_low) >= mutation_threshold_low) {
      mutation_counter_low = i;
      mutation_counter_high += 1;
      t_mutation_count = static_cast<float>(N * 3) / 100;
      fmt::print("\n***Low threshold met! Mutating by 3%... {0}/{1}***\n",
                 mutation_counter_high,
                 mutation_threshold_high);
      fmt::print("iteration: {0}, iteration-delta: {1}\n", i, i - mutation_gap);
      for (auto& critter : critters) {
        fmt::print("critter fitness: {0:.2f}, max-ratio: {1}\n",
                   critter.fitness(),
                   critter.fitness() / max_fitness);
      };
    } else if (mutation_counter_high >= mutation_threshold_high) {
      mutation_counter_high = 0;
      t_mutation_count = static_cast<float>(N * 9) / 100;
      fmt::print("\n***High threshold met! Mutating by 9%...***\n");
    }
    cull_mating_pool(data,
                     pdistb,
                     critters,
                     bucket_count,
                     max_fitness,
                     t_mutation_count,
                     mating_pool_count,
                     top_pool,
                     rng);
    t_mutation_count = mutation_count;
  }
}

int
main(int argc, char** argv)
{
  /*
  Argument parsing of argv.
   */
  cxxopts::Options options("erate", "to optimize data_t");
  options.allow_unrecognised_options().add_options()(
    "i,in_file", "Input file", cxxopts::value<std::string>())(
    "o,out_file", "Output file", cxxopts::value<std::string>())(
    "bucket_count", "Bucket count", cxxopts::value<int>()->default_value("4"))(
    "max_bucket",
    "Max bucket count",
    cxxopts::value<int>()->default_value("150"))(
    "population_count",
    "Population count",
    cxxopts::value<int>()->default_value("100"))(
    "mutation_rate",
    "Mutation rate",
    cxxopts::value<int>()->default_value("1"))(
    "mutation_threshold_low",
    "Mutation threshold low",
    cxxopts::value<int>()->default_value("100000"))(
    "mutation_threshold_high",
    "Mutation threshold high",
    cxxopts::value<int>()->default_value("1000000"))(
    "parent_count", "Parent count", cxxopts::value<int>()->default_value("2"))(
    "mating_pool_count",
    "Mating pool count",
    cxxopts::value<int>()->default_value("10"))(
    "iterations",
    "Iterations",
    cxxopts::value<int>()->default_value("1000000"))(
    "rng_state",
    "Random number generator state",
    cxxopts::value<int>()->default_value("-1"));

  auto result = options.parse(argc, argv);

  /*
CSV parsing of in_file.
 */

  std::vector<erate_t> erate_data;
  std::string lea_number;
  int discount, cost, no_mutate, bucket;

  io::CSVReader<5> in(result["in_file"].as<std::string>());
  in.read_header(io::ignore_extra_column,
                 "lea-number",
                 "discount",
                 "cost",
                 "no-mutate",
                 "bucket");

  while (in.read_row(lea_number, discount, cost, no_mutate, bucket)) {
    erate_data.push_back(
      erate_t{lea_number, discount, cost, no_mutate, bucket});
  }

  /*
  Optimizing function of erate_data.
   */
  auto ts = std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::system_clock::now().time_since_epoch());

  int _rng_state = result["rng_state"].as<int>();
  uint64_t rng_state =
    _rng_state > 0 ? static_cast<uint64_t>(_rng_state) : ts.count();

  optimize_buckets(erate_data,
                   result["out_file"].as<std::string>(),
                   result["bucket_count"].as<int>(),
                   result["max_bucket"].as<int>(),
                   result["population_count"].as<int>(),
                   result["mutation_rate"].as<int>(),
                   result["mutation_threshold_low"].as<int>(),
                   result["mutation_threshold_high"].as<int>(),
                   result["parent_count"].as<int>(),
                   result["mating_pool_count"].as<int>(),
                   result["iterations"].as<int>(),
                   rng_state);
}