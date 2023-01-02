#pragma once


template <class T>
std::pair<performance_counters, performance_counters>
time_it_ns(uint32_t size, T  &&function, size_t repeat) {
  performance_counters agg_min{1e300};
  performance_counters agg_avg{0.0};
  // warm up the cache:
  /*
  for (size_t i = 0; i < 10; i++) {
    function();
  }*/
  for (size_t i = 0; i < repeat; i++) {
    performance_counters start = get_counters();
    function();
    performance_counters end = get_counters();
    performance_counters diff = end - start;
    agg_min = agg_min.min(diff);
    agg_avg += diff;
  }
  agg_avg /= size * repeat;
  agg_min /= size;
  return std::make_pair(agg_min, agg_avg);
}


/*
void process(std::vector<std::string> &lines, size_t volume) {
  size_t repeat = 100;
  double volumeMB = volume / (1024. * 1024.);
  std::cout << "volume = " << volumeMB << " MB " << std::endl;
  pretty_print(volume, lines.size(), "strtod",
               time_it_ns(lines, findmax_strtod, repeat));
  printf("\n");
  pretty_print(volume, lines.size(), "fastfloat",
               time_it_ns(lines, findmax_fastfloat, repeat));
  printf("\n");
}
*/

