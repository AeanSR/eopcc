#include <variant>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>
#include <queue>
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <typeinfo>
#include <typeindex>
#include <cmath>

using namespace std::string_literals;

#define in +in_recieve_set_t()*
#define contains *in_recieve_set_t()+
#define cc(...) std::set<decltype(__VA_ARGS__)>({__VA_ARGS__})

template<class T>
struct in_recieve_key_t {
  T set;
  in_recieve_key_t(T set) : set(set) { }
};

struct in_recieve_set_t {

};

template<class T>
in_recieve_key_t<T> operator*(in_recieve_set_t lhs, T rhs) {
  return in_recieve_key_t<T>(rhs);
}
template<class T>
in_recieve_key_t<T> operator*(T lhs, in_recieve_set_t rhs) {
  return in_recieve_key_t<T>(lhs);
}
template<class T1, class T2>
bool operator+(in_recieve_key_t<T1> lhs, T2 rhs) {
  return lhs.set.find(rhs) != lhs.set.end();
}
template<class T1, class T2>
bool operator+(T2 lhs, in_recieve_key_t<T1> rhs) {
  return rhs.set.find(lhs) != rhs.set.end();
}

#define is %is_recieve_ptr_t()%
struct is_recieve_ptr_t {
  std::type_index type1;
  std::type_index type2;
  operator bool() const { return type1 == type2; }
  is_recieve_ptr_t() : type1(typeid(void)), type2(typeid(void)) {  };
  template<class T> is_recieve_ptr_t operator%(std::shared_ptr<T> rhs)
    { is_recieve_ptr_t ret(*this); return ret.type2 = std::type_index(typeid(*rhs)), ret; }
  is_recieve_ptr_t operator%(const std::type_info& rhs)
    { is_recieve_ptr_t ret(*this); return ret.type2 = std::type_index(rhs), ret; }
};
template<class T> is_recieve_ptr_t operator%(std::shared_ptr<T> lhs, is_recieve_ptr_t rhs)
  { is_recieve_ptr_t ret(rhs); return ret.type1 = std::type_index(typeid(*lhs)), ret; }
is_recieve_ptr_t operator%(const std::type_info& lhs, is_recieve_ptr_t rhs)
  { is_recieve_ptr_t ret(rhs); return ret.type1 = std::type_index(lhs), ret; }

// ===== CPULESS CHARACTERISTICS ================================================================

constexpr double frequency = 1e9; // hertz
constexpr int64_t line_bytes = 64; // bytes

  // -- DDR ----
  // 45nm, 32GB DRAM, 64 Bytes/Line
constexpr double ddr_bus_wire_length = 1e2 * 1e-3; // mm
constexpr double ddr_read_latency = (50.6765 + 0.0493282 * ddr_bus_wire_length) * 1e-9; // ns
constexpr double ddr_write_latency = ddr_read_latency; // ns
constexpr double ddr_leakage = (26201.4 + 317.975) * 1e-3; // mW
constexpr double ddr_read_energy = 267.889 * 1e-9; // nJ
constexpr double ddr_write_energy = 268.068 * 1e-9; // nJ
constexpr double ddr_bandwidth = 34.12 * 1024 * 1024 * 1024; // byte/s

  // -- CACHE ----
  // 45nm, 8KB SRAM, 64 Bytes/Line, Full Associate, 0.219241mm2
constexpr int64_t cache_line = 64; // bytes
constexpr int64_t cache_num_lines = 128; // lines
constexpr double cache_hit_latency = 0.140499 * 1e-9; // ns
constexpr double cache_miss_latency = 0.113233 * 1e-9; // ns
constexpr double cache_write_latency = 0.048177 * 1e-9; // ns
constexpr double cache_leakage = 0.866498 * 1e-3; // mW
constexpr double cache_hit_energy = 0.0557851 * 1e-9; // nJ
constexpr double cache_miss_energy = 0.0557851 * 1e-9; // nJ
constexpr double cache_write_energy = 0.0542248 * 1e-9; // nJ

  // -- SPM ----
  // 45nm, 1MB SRAM, 64 Bytes/Line, 2.88368mm2
constexpr double spm_read_latency = 0.831102 * 1e-9; // ns
constexpr double spm_write_latency = 0.831102 * 1e-9; // ns
constexpr double spm_leakage = 17.7136 * 1e-3; // mW
constexpr double spm_read_energy = 0.0561984 * 1e-9; // nJ
constexpr double spm_write_energy = 0.0555022 * 1e-9; // nJ

constexpr double other_power = 1.5752; // W

constexpr int64_t spm_size = 1024 * 1024;
constexpr int64_t vq_depth = 8;

// ================================================================ CPULESS CHARACTERISTICS =====

using timestamp_t = double;
timestamp_t global_time = 0;
#define ellapse(t) (global_time+(t))
#define posedge(t) [](timestamp_t _t){return ellapse(((_t)*frequency-std::floor((_t)*frequency))/frequency);}(t)

//#define log(...) if(global_time >= 0.){std::cout << "[" << global_time << "] " << __VA_ARGS__ << std::endl;}else
#define log(...)

struct stat_t {
  double ddr_dyne = .0;
  double cache_dyne = .0;
  double spm_dyne = .0;
  double ddr_stae = .0;
  double cache_stae = .0;
  double spm_stae = .0;
  double other_stae = .0;
  timestamp_t time_breakdown[2][2][2];
  timestamp_t total_time;
  int64_t ddr_traffic = 0;
  int64_t cache_hit_count = 0;
  int64_t cache_miss_count = 0;
  int64_t spm_traffic = 0;
  int64_t udet_mem = 0;
  int64_t udet_jmp = 0;

  void stationary(bool ddr_sleep, bool ppu_sleep, bool spu_sleep, timestamp_t start, timestamp_t stop) {
    ddr_stae += ddr_leakage * (stop - start);
    cache_stae += cache_leakage * (stop - start);
    spm_stae += spm_leakage * (stop - start);
    other_stae += other_power * (stop - start);
    time_breakdown[!ddr_sleep][!ppu_sleep][!spu_sleep] += stop - start;
    total_time = stop;
  }
  void ddr_read(int64_t size) {
    ddr_traffic += size;
    ddr_dyne += ddr_read_energy * ((size + line_bytes - 1) / line_bytes);
  }
  void ddr_write(int64_t size) {
    ddr_traffic += size;
    ddr_dyne += ddr_write_energy * ((size + line_bytes - 1) / line_bytes);
  }
  void cache_hit() {
    cache_hit_count++;
    cache_dyne += cache_hit_energy;
  }
  void cache_miss() {
    cache_miss_count++;
    cache_dyne += cache_miss_energy;
  }
  void cache_write() {
    cache_dyne += cache_write_energy;
  }
  void spm_read(int64_t size) {
    spm_traffic += size;
    spm_dyne += spm_read_energy * ((size + line_bytes - 1) / line_bytes);
  }
  void spm_write(int64_t size) {
    spm_traffic += size;
    spm_dyne += spm_write_energy * ((size + line_bytes - 1) / line_bytes);
  }
  void report(std::ostream& os) {
    os << "Energy: " << ddr_dyne + cache_dyne + spm_dyne + ddr_stae + cache_stae + spm_stae + other_stae << std::endl;
    os << "  |- Offchip energy: " << ddr_dyne + ddr_stae << std::endl;
    os << "  |    |- DDR Dynamic: " << ddr_dyne << std::endl;
    os << "  |    -- DDR Static: " << ddr_stae << std::endl;
    os << "  |- Onchip energy: " << cache_dyne + spm_dyne + cache_stae + spm_stae + other_stae << std::endl;
    os << "       |- Cache: " << cache_dyne + cache_stae << std::endl;
    os << "       |    |- Cache Dynamic: " << cache_dyne << std::endl;
    os << "       |    -- Cache Static: " << cache_stae << std::endl;
    os << "       |- ScratchPad: " << spm_dyne + spm_stae << std::endl;
    os << "       |    |- ScratchPad Dynamic: " << spm_dyne << std::endl;
    os << "       |    -- ScratchPad Static: " << spm_stae << std::endl;
    os << "       -- Others: " << other_stae << std::endl;
    os << std::endl;
    os << "Time: " << total_time << std::endl;
    timestamp_t time_ddr = .0, time_ppu = .0, time_spu = .0;
    timestamp_t time_ddr_exclusive = .0, time_compute = .0;
    for (int d = 0; d < 2; d++) for (int p = 0; p < 2; p++) for (int s = 0; s < 2; s++) {
      const char * punc[] = { "-", "+" };
      os << "  " << punc[d] << "DDR," << punc[p] << "PPU," << punc[s] << "SPU: " << time_breakdown[d][p][s] << "(" << (int)(10000 * (time_breakdown[d][p][s] / total_time) + 0.5) / 100. << "%)" << std::endl;
      if (d) time_ddr += time_breakdown[d][p][s];
      if (p) time_ppu += time_breakdown[d][p][s];
      if (s) time_spu += time_breakdown[d][p][s];
      if (d && !p && !s) time_ddr_exclusive += time_breakdown[d][p][s];
      if (p || s) time_compute += time_breakdown[d][p][s];
    }
    os << "  +DDR: " << time_ddr << "(" << (int)(10000 * (time_ddr / total_time) + 0.5) / 100. << "%)" << std::endl;
    os << "  +PPU: " << time_ppu << "(" << (int)(10000 * (time_ppu / total_time) + 0.5) / 100. << "%)" << std::endl;
    os << "  +SPU: " << time_spu << "(" << (int)(10000 * (time_spu / total_time) + 0.5) / 100. << "%)" << std::endl;
    os << "  +DDR,-PPU,-SPU: " << time_ddr_exclusive << "(" << (int)(10000 * (time_ddr_exclusive / total_time) + 0.5) / 100. << "%)" << std::endl;
    os << "  +PPU/+SPU: " << time_compute << "(" << (int)(10000 * (time_compute / total_time) + 0.5) / 100. << "%)" << std::endl;
    os << std::endl;
    os << "Cache Accesses: " << cache_hit_count + cache_miss_count << std::endl;
    os << "Cache Hit: " << cache_hit_count << "(" << (int)(10000 * ((double)cache_hit_count / (cache_hit_count + cache_miss_count)) + 0.5) / 100. << "%)" << std::endl;
    os << "Cache Miss: " << cache_miss_count << "(" << (int)(10000 * ((double)cache_miss_count / (cache_hit_count + cache_miss_count)) + 0.5) / 100. << "%)" << std::endl;
    os << "DDR Traffic: " << ddr_traffic << std::endl;
    os << "SPM Traffic: " << spm_traffic << std::endl;
    os << std::endl;
    os << "Undetermined Memory Accesses: " << udet_mem << std::endl;
    os << "Undetermined Conditional Jump: " << udet_jmp << std::endl;
  }

};

stat_t& stat() {
  static stat_t _stat;
  return _stat;
}

struct coroutine_t {
  int64_t _crbp;
  bool sleep;
#define yield(ts) do{_crbp=__LINE__;eq.emplace(ts,this);return;case __LINE__:;}while(0)
#define awake do{if(sleep){sleep=false;eq.emplace(ellapse(0),this);}}while(0)
#define hibernate do{_crbp=__LINE__;sleep=true;return;case __LINE__:;}while(0)
#define async(...) void operator()(){sleep=false;switch(_crbp){case 0: default:{__VA_ARGS__}}}
  virtual void operator()() = 0;
  static std::list<coroutine_t*>& list() {
    static std::list<coroutine_t*> _lcr;
    return _lcr;
  }
  coroutine_t() : _crbp(0), sleep(true) { list().push_back(this); }
};

struct future_t {
  bool finished = false;
  std::vector<coroutine_t*> _ftcb;
#define await(fut) do{if([&](auto&&_fut){if(!_fut||_fut->finished)return false;_fut->_ftcb.push_back(this);return true;}(fut)){_crbp=__LINE__;return;}case __LINE__:;}while(0)
#define finish(fut) [&](auto&&_fut){if(!_fut)return;for(auto&&p:_fut->_ftcb)eq.emplace(ellapse(0),p);_fut->finished=true;}(fut)
#define finish_at(fut,ts) do{future_scheduler().schedule(ts,fut);}while(0)
#define link(fut, cond) do { future_links.emplace_back(); future_links.back()._fut = (fut); future_links.back()._cond = (cond); future_links.back()(); } while(0)
  using ptr = std::shared_ptr<future_t>;
  static ptr new_() { return std::make_shared<future_t>(); }
};

struct event_t {
  timestamp_t time;
  coroutine_t* callback;
  event_t(timestamp_t time, coroutine_t* callback) : time(time), callback(callback) { }
  bool operator<(const event_t& rhs) const {
    return time > rhs.time;
  }
};

std::priority_queue<event_t> eq;

struct future_link_t : public coroutine_t {
  future_t::ptr _fut;
  future_t::ptr _cond;
  async(
    await(_cond);
    finish(_fut);
  )
};

std::list<future_link_t> future_links;

struct future_scheduler_t : public coroutine_t {
  std::multimap<timestamp_t, future_t::ptr> scheduled;
  void schedule(timestamp_t t, future_t::ptr f) {
    scheduled.insert({t,f}); eq.emplace(t, this);
  }
  async(
    while(1) {
      for (auto i = scheduled.begin(); i != scheduled.upper_bound(global_time); i++)
        finish(i->second);
      scheduled.erase(scheduled.begin(), scheduled.upper_bound(global_time));
      hibernate;
    }
  )
};

future_scheduler_t& future_scheduler() {
  static future_scheduler_t _fs;
  return _fs;
}

struct cell_t {
  bool det;
  union {
    int64_t i;
    double f;
  } data;
  cell_t() : det(true), data() { }
  cell_t(int64_t i, bool det = true) : det(det), data({.i=i}) { }
  cell_t(double f, bool det = true) : det(det), data({.f=f}) { }
  bool operator==(const cell_t& rhs) const {
    return det && rhs.det && data.i == rhs.data.i;
  }
  bool operator!=(const cell_t& rhs) const {
    return !(*this == rhs);
  }
};

struct abstract_mem_t {
  virtual cell_t& ref(int64_t addr) = 0;
};

struct mem_t : public abstract_mem_t {
  std::unordered_map<int64_t, cell_t> cells;
  int64_t word_size = 8;
  virtual cell_t& ref(int64_t addr) {
    //addr /= word_size;
    return cells[addr];
  }
  mem_t() = default;
  mem_t(int64_t word_size) : word_size(word_size) { }
};

mem_t& gr() {
  static mem_t _gr(1);
  return _gr;
}

struct spm_t : public abstract_mem_t {
  cell_t cell;
  int64_t word_size = 2;
  virtual cell_t& ref(int64_t addr) {
    cell.det = false;
    return cell;
  }
};

struct io_t : public coroutine_t {
  struct get_request_t {
    timestamp_t ts;
    int64_t addr;
    int64_t size;
    cell_t* data;
    future_t::ptr fut;
    get_request_t(int64_t addr, int64_t size, cell_t* data, future_t::ptr fut) : ts(global_time), addr(addr), size(size), data(data), fut(fut) { }
  };
  struct set_request_t {
    timestamp_t ts;
    int64_t addr;
    int64_t size;
    cell_t* data;
    future_t::ptr fut;
    set_request_t(int64_t addr, int64_t size, cell_t* data, future_t::ptr fut) : ts(global_time), addr(addr), size(size), data(data), fut(fut) { }
  };
  std::list<get_request_t> gets;
  std::list<set_request_t> sets;
  timestamp_t get_finish;
  timestamp_t set_finish;
  async(while(1) {
    while (get_finish || set_finish || !gets.empty() || !sets.empty()) {
      log("io loop.");
      if (!get_finish && !gets.empty() && (duplex() || !set_finish) && (sets.empty() || gets.front().ts <= sets.front().ts)) {
        log("io start get.");
        get_finish = ellapse(get_exec(gets.front()));
      } else if (!set_finish && !sets.empty() && (duplex() || !get_finish) && (gets.empty() || sets.front().ts <= gets.front().ts)) {
        log("io start set.");
        set_finish = ellapse(set_exec(sets.front()));
      } else if (get_finish && (!set_finish || get_finish <= set_finish)) {
        log("io yield get.");
        while (get_finish > global_time)
          yield(get_finish);
        get_finish = 0;
        log("io get data arrive.");
        if (gets.front().data) *gets.front().data = mem()->ref(gets.front().addr);
        log("io get finish.");
        finish(gets.front().fut);
        gets.pop_front();
      } else if (set_finish && (!get_finish || get_finish > set_finish)) {
        log("io yield set.");
        while (set_finish > global_time)
          yield(set_finish);
        set_finish = 0;
        log("io set data arrive: " << sets.front().addr << ", " << sets.front().data);
        if (sets.front().data) log("data: " << (*sets.front().data).data.i);
        if (sets.front().data)
          mem()->ref(sets.front().addr) = *sets.front().data;
        else
          mem()->ref(sets.front().addr).det = false;
        log("io set finish.");
        finish(sets.front().fut);
        sets.pop_front();
      }
    }
    log("io hibernate.");
    hibernate;
    log("io awake latency.");
    if (!gets.empty())
      yield(ellapse(read_latency()));
    else
      yield(ellapse(write_latency()));
  })
  future_t::ptr get(int64_t addr, int64_t size) {
    future_t::ptr fut = future_t::new_();
    gets.emplace_back(addr, size, nullptr, fut); awake;
    return fut;
  }
  future_t::ptr set(int64_t addr, int64_t size) {
    future_t::ptr fut = future_t::new_();
    sets.emplace_back(addr, size, nullptr, fut); awake;
    return fut;
  }
  future_t::ptr get(int64_t addr, cell_t* data) {
    future_t::ptr fut = future_t::new_();
    if (sleep) {
      gets.emplace_back(addr, line_bytes, data, fut); awake;
      return fut;
    } else {
      *data = mem()->ref(addr);
      auto time = get_exec(get_request_t(addr, line_bytes, data, fut));
      if (get_finish)
        get_finish += time;
      finish_at(fut, ellapse(time));
      return fut;
    }
  }
  future_t::ptr set(int64_t addr, cell_t* data) {
    future_t::ptr fut = future_t::new_();
    if (sleep) {
      sets.emplace_back(addr, line_bytes, data, fut); awake;
      return fut;
    } else {
      mem()->ref(addr) = *data;
      auto time = set_exec(set_request_t(addr, line_bytes, data, nullptr));
      if (set_finish)
        set_finish += time;
      finish_at(fut, ellapse(time));
      return fut;
    }
  }
  virtual bool duplex() const = 0;
  virtual abstract_mem_t* mem() = 0;
  virtual timestamp_t get_exec(get_request_t req) = 0;
  virtual timestamp_t set_exec(set_request_t req) = 0;
  virtual timestamp_t read_latency() const = 0;
  virtual timestamp_t write_latency() const = 0;
};

struct mem_io_t : public io_t {
  mem_t data_array;
  virtual bool duplex() const { return false; }
  virtual abstract_mem_t* mem() { return &data_array; }
  virtual timestamp_t get_exec(get_request_t req) {
    stat().ddr_read(req.size);
    return ((req.size + line_bytes - 1) / line_bytes) * line_bytes / ddr_bandwidth;
  }
  virtual timestamp_t set_exec(set_request_t req) {
    stat().ddr_write(req.size);
    return ((req.size + line_bytes - 1) / line_bytes) * line_bytes / ddr_bandwidth;
  }
  virtual timestamp_t read_latency() const { return ddr_read_latency; }
  virtual timestamp_t write_latency() const { return ddr_write_latency; }
};

mem_io_t& ddr() {
  static mem_io_t _io;
  return _io;
}

struct spm_io_t : public io_t {
  spm_t data_array;
  virtual bool duplex() const { return true; }
  virtual abstract_mem_t* mem() { return &data_array; }
  virtual timestamp_t get_exec(get_request_t req) {
    stat().spm_read(req.size);
    return (req.size + line_bytes - 1) / line_bytes / frequency / 2.;
  }
  virtual timestamp_t set_exec(set_request_t req) {
    stat().spm_write(req.size);
    return (req.size + line_bytes - 1) / line_bytes / frequency;
  }
  virtual timestamp_t read_latency() const { return spm_read_latency; }
  virtual timestamp_t write_latency() const { return spm_write_latency; }
};

spm_io_t& spm(int bank) {
  static spm_io_t _io[2];
  return _io[bank];
}

struct cache_t : public coroutine_t {
  struct _tag_t {
    timestamp_t last_used;
    int64_t tag;
    _tag_t(timestamp_t ts, int64_t tag) : last_used(ts), tag(tag) { }

  };
  struct tag_t {
    std::shared_ptr<_tag_t> ptr;
    tag_t() = default;
    tag_t(timestamp_t ts, int64_t tag) : ptr(std::make_shared<_tag_t>(ts, tag)) { }
    bool operator<(const tag_t& rhs) const {
      return ptr->last_used > rhs.ptr->last_used;
    }
  };
  std::priority_queue<tag_t> history;
  std::unordered_map<int64_t, tag_t> tags;
  mem_t data_array;
  struct cache_request_t {
    int mode;
    int64_t addr;
    int64_t size;
    cell_t* data;
    future_t::ptr fut;
    cache_request_t(int mode, int64_t addr, cell_t* data, future_t::ptr fut) : mode(mode), addr(addr), size(size), data(data), fut(fut) { }
    cache_request_t() = default;
  };
  std::list<cache_request_t> reqs;
  cell_t null_cell;

  bool lookup(int64_t addr) {
    int64_t tag = addr / line_bytes;
    if (tags.find(tag) != tags.end()) {
      return true;
    }
    return false;
  }
  void record(int64_t addr) {
    int64_t tag = addr / line_bytes;
    if (lookup(addr)) {
      tags[tag].ptr->last_used = global_time;
    } else {
      tag_t t(global_time, tag);
      tags[tag] = t;
      history.push(t);
      if (history.size() > cache_num_lines) {
        tags.erase(history.top().ptr->tag);
        history.pop();
      }
    }
  }

  cache_request_t req;
  bool hit;
  async(while(1) {
    while(!reqs.empty()) {
      req = reqs.front();
      reqs.pop_front();

      hit = lookup(req.addr);
      yield(ellapse(hit ? cache_hit_latency : cache_miss_latency));
      if (hit) stat().cache_hit(); else stat().cache_miss();

      if (req.mode == 0) { // read
        if (!hit) { // miss, must read from ddr.
          await(ddr().get(req.addr, &null_cell));
        }
        *req.data = data_array.ref(req.addr);
        finish(req.fut); // return result while writing cache.
        if (!hit) {
          record(req.addr);
          yield(ellapse(cache_write_latency));
        }
      } else { // write
        stat().cache_write();
        finish(req.fut); // execution dont need to wait cache writing.
        if (!hit || *req.data != data_array.ref(req.addr)) {
          log(__LINE__);
          if (!hit) ddr().set(req.addr, req.data); // no await, write cache and ddr simutaneously.
          log(__LINE__);
          yield(ellapse(cache_write_latency));
          data_array.ref(req.addr) = *req.data;
          record(req.addr);
        }
      }
    }
    hibernate;
  })
  future_t::ptr get(int64_t addr, cell_t* data) {
    future_t::ptr fut = future_t::new_();
    reqs.emplace_back(0, addr, data, fut); awake;
    return fut;
  }
  future_t::ptr set(int64_t addr, cell_t* data) {
    future_t::ptr fut = future_t::new_();
    reqs.emplace_back(1, addr, data, fut); awake;
    return fut;
  }
};

cache_t& cache() {
  static cache_t _cache;
  return _cache;
}

struct param_t {
  using ptr = std::shared_ptr<param_t>;
  virtual int64_t eval() const = 0;
  virtual int visualizing() const = 0;
};

struct reg_t : public param_t {
  int64_t regid;
  virtual int64_t eval() const { return regid; }
  friend std::ostream &operator<<(std::ostream &out, const reg_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
  reg_t(int64_t regid) : regid(regid) { }
  reg_t() : regid(0) { }
};

struct imm_t : public param_t {
  ;
};

// int imm_t
struct iimm_t : public imm_t {
  int64_t imm;
  virtual int64_t eval() const { return imm; }
  friend std::ostream &operator<<(std::ostream &out, const iimm_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
  iimm_t(int64_t i) : imm(i) { }
  iimm_t() = default;
};

// float imm_t
struct fimm_t : public imm_t {
  double imm;
  virtual int64_t eval() const { return imm; }
  friend std::ostream &operator<<(std::ostream &out, const fimm_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
  fimm_t(double i) : imm(i) { }
  fimm_t() = default;
};

struct addr_t : public param_t {
  ;
};

struct raddr_t : public addr_t {
  reg_t reg;
  virtual int64_t eval() const { return reg.regid; }
  friend std::ostream &operator<<(std::ostream &out, const raddr_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
  raddr_t(reg_t i) : reg(i) { }
  raddr_t() = default;
};

struct iaddr_t : public addr_t {
  iimm_t imm;
  virtual int64_t eval() const { return imm.eval(); }
  friend std::ostream &operator<<(std::ostream &out, const iaddr_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
  iaddr_t(int64_t i) : imm(i) { }
  iaddr_t() = default;
};

struct output_t : public param_t {
  std::string content;
  // TODO(): print information
  virtual int64_t eval() const { return 0; }
  friend std::ostream &operator<<(std::ostream &out, const output_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
  output_t(std::string i) : content(i) { }
  output_t() = default;
};
// reg_t:   rX
std::ostream & operator << (std::ostream &os, const reg_t &t) {
  std::cout << "r" << t.regid;
  return os;
}
// iimm_t:   #X
std::ostream & operator << (std::ostream &os, const iimm_t &t) {
  std::cout << "#" << t.imm;
  return os;
}
// fimm_t:   #X
std::ostream & operator << (std::ostream &os, const fimm_t &t) {
  std::cout << "#" << t.imm;
  return os;
}
// raddr_t: ptr/rX
std::ostream & operator << (std::ostream &os, const raddr_t &t) {
  std::cout << "ptr/" << t.reg;
  return os;
}
// raddr_t: ptr/#X
std::ostream & operator << (std::ostream &os, const iaddr_t &t) {
  std::cout << "ptr/" << t.imm;
  return os;
}
// output_t: XXX
std::ostream & operator << (std::ostream &os, const output_t &t) {
  std::cout << t.content;
  return os;
}

struct rob_t : public coroutine_t {
  struct lock_t {
    int64_t seq;
    bool read;
    bool operator<(const lock_t& rhs) const { return seq < rhs.seq; }
    lock_t(int64_t seq, bool read) : seq(seq), read(read) { }
  };
  std::multimap<int64_t, lock_t> gr_lock;
  std::multimap<int64_t, lock_t> spm_begins;
  std::multimap<int64_t, lock_t> spm_ends;
  int64_t rob_seq = 1;
  void unlock(int64_t lock_seq) {
    log("rob unlock " << lock_seq << ", current spm_begins: " << spm_begins.size() << ", spm_ends: " << spm_ends.size());
    if (lock_seq > 0) {
      for (auto i = gr_lock.begin(); i != gr_lock.end(); i++) {
        if (i->second.seq == lock_seq) { gr_lock.erase(i); return; }
      }
    } else {
      for (auto i = spm_begins.begin(); i != spm_begins.end(); i++) {
        log("rob spm_begins seq: " << i->second.seq);
        if (i->second.seq == lock_seq) { spm_begins.erase(i); break; }
      }
      for (auto i = spm_ends.begin(); i != spm_ends.end(); i++) {
        log("rob spm_ends seq: " << i->second.seq);
        if (i->second.seq == lock_seq) { spm_ends.erase(i); return; }
      }
    }
    log("rob after unlock " << lock_seq << ", spm_begins: " << spm_begins.size() << ", spm_ends: " << spm_ends.size());
  }

  struct rob_request_t {
    int64_t lock_seq;
    future_t::ptr fut;
    rob_request_t(int64_t lock_seq, future_t::ptr fut) : lock_seq(lock_seq), fut(fut) { }
  };
  std::list<rob_request_t> reqs;
  future_t::ptr fin = future_t::new_();
  async(while(1){
    while(!reqs.empty()) {
      await(reqs.front().fut);
      //std::cout << "unlock " << reqs.front().fut << std::endl;
      unlock(reqs.front().lock_seq);
      reqs.pop_front();
      finish(fin);
      log("rob finish fin");
      fin = future_t::new_();
    }
    hibernate;
  })
  bool test_read(reg_t r) const {
    return std::all_of(gr_lock.lower_bound(r.regid), gr_lock.upper_bound(r.regid), [](auto&& p){return p.second.read;});
  }
  bool test_write(reg_t r) const {
    return std::none_of(gr_lock.lower_bound(r.regid), gr_lock.upper_bound(r.regid), [](auto&& p){return true;});
  }
  bool test_read(int64_t start, int64_t end) const {
    std::multimap<int64_t, lock_t> deps;
    std::set_intersection(
        spm_begins.begin(), spm_begins.lower_bound(end),
        spm_ends.upper_bound(start), spm_ends.end(),
        std::inserter(deps, deps.end()), [](auto&& p1, auto&& p2){ return p1.second < p2.second; }
    );
    return deps.empty() || std::all_of(deps.begin(), deps.end(), [](auto&& p){return p.second.read;});
  }
  bool test_write(int64_t start, int64_t end) const {
    std::multimap<int64_t, lock_t> deps;
    //for (auto&& b : spm_begins) std::cout << b.first << ",";
    //std::cout << ":" << std::distance(spm_begins.begin(), spm_begins.lower_bound(end)) << std::endl;
    
    std::set_intersection(
        spm_begins.begin(), spm_begins.lower_bound(end),
        spm_ends.upper_bound(start), spm_ends.end(),
        std::inserter(deps, deps.end()), [](auto&& p1, auto&& p2){ return p1.second < p2.second; }
    );
    return deps.empty();
  }
/*  bool test_read(int64_t start, int64_t end) const {
    bool ret = _test_read(start, end);
    //std::cout << "test_read(" << start << "," << end << ")=" << ret << std::endl;
    return ret;
  }
  bool test_write(int64_t start, int64_t end) const {
    bool ret = _test_write(start, end);
    //std::cout << "test_write(" << start << "," << end << ")=" << ret << std::endl;
    return ret;
  }*/
  void lock_read(future_t::ptr fut, reg_t r) {
    gr_lock.emplace(r.regid, lock_t(rob_seq, true));
    reqs.emplace_back(rob_seq, fut);
    rob_seq++; awake;
  }
  void lock_write(future_t::ptr fut, reg_t r) {
    gr_lock.emplace(r.regid, lock_t(rob_seq, false));
    reqs.emplace_back(rob_seq, fut);
    rob_seq++; awake;
  }
  void lock_read(future_t::ptr fut, int64_t start, int64_t end) {
    //std::cout << "lock_read(" << fut << "," << start << "," << end << ")" << std::endl;
    spm_begins.emplace(start, lock_t(-rob_seq, true));
    spm_ends.emplace(end, lock_t(-rob_seq, true));
    //for (auto&& b : spm_begins) std::cout << b.first << ","; std::cout << std::endl;
    reqs.emplace_back(-rob_seq, fut);
    rob_seq++; awake;
  }
  void lock_write(future_t::ptr fut, int64_t start, int64_t end) {
    //std::cout << "lock_write(" << fut << "," << start << "," << end << ")" << std::endl;
    spm_begins.emplace(start, lock_t(-rob_seq, false));
    spm_ends.emplace(end, lock_t(-rob_seq, false));
    //for (auto&& b : spm_begins) std::cout << b.first << ","; std::cout << std::endl;
    reqs.emplace_back(-rob_seq, fut);
    rob_seq++; awake;
  }
};

rob_t& rob() {
  static rob_t _rob;
  return _rob;
}

struct spu_t : public coroutine_t {
  struct spu_request_t {
    std::string op;
    cell_t* dest;
    cell_t lhs;
    cell_t rhs;
    future_t::ptr fut;
    spu_request_t() = default;
    spu_request_t(std::string op, cell_t* dest, cell_t lhs, cell_t rhs, future_t::ptr fut) : op(op), dest(dest), lhs(lhs), rhs(rhs), fut(fut) { }
  };
  virtual int exec(spu_request_t& req) {
    std::unordered_map<std::string, int(*)(cell_t*,cell_t,cell_t)> ops {
      { "cvtif"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.i, 1; } },
      { "cvtfi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.f, 1; } },
      { "muli"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i * r.data.i, 1; } },
      { "mulf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f * r.data.f, 1; } },
      { "divi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i / r.data.i, 1; } },
      { "divf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f / r.data.f, 1; } },
      { "modi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i % r.data.i, 1; } },
      { "addi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i + r.data.i, 1; } },
      { "addf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f + r.data.f, 1; } },
      { "subi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i - r.data.i, 1; } },
      { "subf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f - r.data.f, 1; } },
      { "shli"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i << r.data.i, 1; } },
      { "shri"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i >> r.data.i, 1; } },
      { "lti"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i < r.data.i, 1; } },
      { "ltf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f < r.data.f, 1; } },
      { "gti"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i > r.data.i, 1; } },
      { "gtf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f > r.data.f, 1; } },
      { "lei"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i <= r.data.i, 1; } },
      { "lef"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f <= r.data.f, 1; } },
      { "gei"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i >= r.data.i, 1; } },
      { "gef"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f >= r.data.f, 1; } },
      { "eqi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i == r.data.i, 1; } },
      { "eqf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f == r.data.f, 1; } },
      { "nei"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i != r.data.i, 1; } },
      { "nef"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.f = l.data.f != r.data.f, 1; } },
      { "andi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i & r.data.i, 1; } },
      { "xori"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i ^ r.data.i, 1; } },
      { "ori"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i | r.data.i, 1; } },
      { "noti"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = ~l.data.i, 1; } },
      { "movis"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i, 1; } },
      { "movfs"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i, 1; } },
      { "jz"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i ? d->data.i : r.data.i, 1; } },
    };
    req.dest->det = req.lhs.det && req.rhs.det;
    log("spu exec before: " << req.op << "," << req.dest->data.i << "," << req.lhs.data.i << "," << req.rhs.data.i);
    int cycles = ops[req.op](req.dest, req.lhs, req.rhs);
    log("spu exec after: " << req.op << "," << req.dest->data.i << "," << req.lhs.data.i << "," << req.rhs.data.i);
    return cycles;
  }
  std::list<spu_request_t> reqs;
  async(
    while(1) {
      while(!reqs.empty()) {
        yield(posedge(ellapse(exec(reqs.front()) * (1. / frequency))));
        finish(reqs.front().fut);
        reqs.pop_front();
      }
      hibernate;
    }
  )
  future_t::ptr issue(std::string op, cell_t* dest, cell_t lhs, cell_t rhs) {
    future_t::ptr fut = future_t::new_();
    reqs.emplace_back(op, dest, lhs, rhs, fut); awake;
    return fut;
  }
};

spu_t& spu() {
  static spu_t _spu;
  return _spu;
}

struct ppu_t : public coroutine_t {
  struct ppu_request_t {
    std::string op;
    std::vector<int64_t> size;
    future_t::ptr fut;
    ppu_request_t() = default;
    ppu_request_t(std::string op, std::vector<int64_t> size, future_t::ptr fut) : op(op), size(size), fut(fut) { }
  };
  enum { NONE, UNARY, BINARY, REDUCE, POOL, CONV, MM, MOVSV };
  int mode = NONE;
  int64_t cycles = 0;
  int64_t total_cycles = 0;
  std::list<ppu_request_t> reqs;
  std::list<future_t::ptr> hs;
  async(
    while(1) {
      while(!reqs.empty()) {
        yield(posedge(global_time));
        if (reqs.front().op in cc("movsv"s)) {
          mode = MOVSV;
          cycles = 1;
          total_cycles = 1;
        } else if (reqs.front().op in cc("movv"s, "act"s, "floor"s, "mulvf"s, "divvf"s, "addvf"s, "subvf"s, "subfv"s,
                             "ltvf"s, "gtvf"s, "levf"s, "gevf"s, "eqvf"s, "nevf"s)) {
          mode = UNARY;
          cycles = total_cycles = (reqs.front().size.back() + line_bytes - 1) / line_bytes;
        } else if (reqs.front().op in cc("mulv"s, "divv"s, "addv"s, "subv"s,
                             "ltv"s, "gtv"s, "lev"s, "gev"s, "eqv"s, "nev"s)) {
          mode = BINARY;
          cycles = total_cycles = (reqs.front().size.back() + line_bytes - 1) / line_bytes;
        } else if (reqs.front().op in cc("cycleadd"s)) {
          mode = BINARY;
          cycles = total_cycles = (reqs.front().size[3] + line_bytes - 1) / line_bytes;
        } else if (reqs.front().op in cc("haddv"s, "hmulv"s, "hminv"s, "hmaxv"s)) {
          mode = REDUCE;
          cycles = total_cycles = (reqs.front().size.back() + line_bytes - 1) / line_bytes;
        } else if (reqs.front().op in cc("pool"s)) {
          mode = POOL;
          int64_t fi = (reqs.front().size[2] * 2 + line_bytes - 1) / line_bytes;
          int64_t kx = reqs.front().size[3];
          int64_t ky = reqs.front().size[4];
          int64_t sx = reqs.front().size[5];
          int64_t sy = reqs.front().size[6];
          int64_t xi = reqs.front().size[7];
          int64_t yi = reqs.front().size[8];
          int64_t bt = reqs.front().size[9];
          int64_t px = reqs.front().size[10];
          int64_t py = reqs.front().size[11];
          int64_t xo = (xi - kx + px * 2 + sx) / sx;
          int64_t yo = (yi - ky + py * 2 + sy) / sy;
          cycles = total_cycles = bt * xo * yo * fi * kx * ky;
        } else if (reqs.front().op in cc("conv"s)) {
          mode = CONV;
          int64_t fi = (reqs.front().size[3] * 2 + line_bytes - 1) / line_bytes;
          int64_t fo = (reqs.front().size[4] * 2 + line_bytes - 1) / line_bytes;
          int64_t kx = reqs.front().size[5];
          int64_t ky = reqs.front().size[6];
          int64_t xi = reqs.front().size[7];
          int64_t yi = reqs.front().size[8];
          int64_t bt = reqs.front().size[9];
          int64_t sx = reqs.front().size[10];
          int64_t sy = reqs.front().size[11];
          int64_t px = reqs.front().size[12];
          int64_t py = reqs.front().size[13];
          int64_t xo = (xi - kx + px * 2 + sx) / sx;
          int64_t yo = (yi - ky + py * 2 + sy) / sy;
          cycles = total_cycles = bt * xo * yo * fi * fo * kx * ky;
        } else if (reqs.front().op in cc("mm"s)) {
          mode = MM;
          int64_t fi = (reqs.front().size[4] * 2 + line_bytes - 1) / line_bytes;
          int64_t fo = (reqs.front().size[5] * 2 + line_bytes - 1) / line_bytes;
          int64_t ni = reqs.front().size[6];
          int64_t bt = reqs.front().size[7];
          //std::cout << "mm" << bt << "," << ni << "," << fi << "," << fo << std::endl;
          cycles = total_cycles = bt * ni * fi * fo;
        } else if (reqs.front().op in cc("trans"s)) {
          mode = UNARY;
          int64_t n1 = (reqs.front().size[2] * 2 + line_bytes - 1) / line_bytes;
          int64_t n2 = (reqs.front().size[3] * 2 + line_bytes - 1) / line_bytes;
          cycles = total_cycles = n1 * n2;
        }
        // pipeline 3: wb
        if (mode == NONE) {
        } else if (mode in std::set<int>{REDUCE}) {
        } else if (mode in std::set<int>{UNARY, BINARY, POOL, CONV, MM, MOVSV}) {
            hs.push_back(spm(0).set(0, cycles * line_bytes));
        }
        // pipeline 2: ex
        hs.push_back(future_t::new_());
        finish_at(hs.back(), ellapse(cycles / frequency));
        // pipeline 1: ld
        if (mode == NONE) {
        } else if (mode in std::set<int>{UNARY, REDUCE}) {
            hs.push_back(spm(0).get(0, cycles * line_bytes));
        } else if (mode in std::set<int>{BINARY, POOL, CONV, MM}) {
            hs.push_back(spm(0).get(0, cycles * line_bytes));
            hs.push_back(spm(0).get(1, cycles * line_bytes));
        }
        // pipeline 0: ctrl

        //std::cout << "ppu working..." << std::endl;
        while(!hs.empty()) {
          await(hs.front());
          hs.pop_front();
        }
        cycles = total_cycles = 0;
        finish(reqs.front().fut);
        reqs.pop_front();
        //std::cout << "ppu insts to process " << reqs.size() << std::endl;
      }
      //std::cout << "ppu hibernated!" << std::endl;
      hibernate;
      yield(ellapse(2.0 / frequency));
    })

  typename future_t::ptr issue(std::string op, std::vector<param_t::ptr> args) {
    future_t::ptr fut = future_t::new_();
    std::vector<int64_t> size;
    for (auto&& p : args) size.push_back(p->eval());
    reqs.emplace_back(op, size, fut); awake;
    return fut;
  }
};

ppu_t& ppu() {
  static ppu_t _ppu;
  return _ppu;
}

struct pdma_t : public coroutine_t {
  struct pdma_request_t {
    int64_t addr;
    int64_t size;
    bool load;
    future_t::ptr fut;
    pdma_request_t(int64_t addr, int64_t size, bool load, future_t::ptr fut) : addr(addr), size(size), load(load), fut(fut) { }
  };
  std::list<pdma_request_t> reqs;
  std::list<future_t::ptr> hs;
  async(while(1) {
    while(!reqs.empty()) {
      yield(posedge(global_time));
      if (reqs.front().load) {
        hs.push_back(spm(1).set(0, reqs.front().size));
        hs.push_back(ddr().get(reqs.front().addr, reqs.front().size));
      } else {
        hs.push_back(spm(1).get(0, reqs.front().size));
          log(__LINE__);
        hs.push_back(ddr().set(reqs.front().addr, reqs.front().size));
          log(__LINE__);
      }
      while (!hs.empty()) {
        await(hs.front());
        hs.pop_front();
      }
      finish(reqs.front().fut);
      reqs.pop_front();
    }
    hibernate;
  })
  future_t::ptr issue(std::string op, std::vector<param_t::ptr> args) {
    future_t::ptr fut = future_t::new_();
    if (op == "loadv"s) {
      reqs.emplace_back(args[1]->eval(), args[2]->eval(), true, fut);
    } else if (op == "storev"s) {
      reqs.emplace_back(args[0]->eval(), args[2]->eval(), false, fut);
    } else if (op == "strideio"s) {
      bool load = args[0]->eval() < spm_size;
      /*for (int i = 0; i < args[4]->eval(); i++) {
        reqs.emplace_back(args[load ? 0 : 1]->eval() + i * args[3]->eval(), args[2]->eval(), load,
                          i == args[4]->eval() - 1 ? fut : nullptr);
      }*/
      reqs.emplace_back(args[load ? 0 : 1]->eval(), args[2]->eval() * args[4]->eval(), load, fut);
    }
    awake;
    return fut;
  }
};

pdma_t& pdma() {
  static pdma_t _pdma;
  return _pdma;
}

struct inst_t {
  int pc;
  std::string op;
  std::vector<param_t::ptr> args;
  inst_t(int pc_, std::string op_, std::vector<param_t::ptr> args_) : pc(pc_), op(op_), args(args_) {}
  inst_t() = default;
};

struct vq_t : public coroutine_t {
  std::list<std::pair<inst_t, future_t::ptr>> q;
  inst_t i;
  future_t::ptr h;
  std::vector<int64_t> sizes;
  async(while(1){                 
    while(!q.empty()) {
      i = q.front().first;
      if (i.op in cc("strideio"s, "loadv"s, "storev"s))
        h = pdma().issue(i.op, i.args);
      else
        h = ppu().issue(i.op, i.args);
      link(q.front().second, h);
      q.pop_front();
      yield(ellapse(1. / frequency));
    }
    hibernate;
  })
  future_t::ptr issue(inst_t i) {
    q.emplace_back(i, future_t::new_());
    awake;
    return q.back().second;
  }
  bool full() const {
    return q.size() >= vq_depth;
  }
  bool empty() const {
    return q.empty();
  }
};

vq_t& vq() {
  static vq_t _vq;
  return _vq;
}

struct rob_test_t {
  int64_t start;
  int64_t end;
  reg_t reg;
  bool is_addr;
  bool is_reg;
  bool read;
  rob_test_t(bool read, int64_t start, int64_t end) : start(start), end(end), reg(0), is_addr(0 <= start && start < spm_size && 0 < end && end <=spm_size), is_reg(false), read(read) { }
  rob_test_t(bool read, reg_t reg) : start(0), end(0), reg(reg), is_addr(false), is_reg(reg.eval() > 0), read(read) { }
  rob_test_t() : is_addr(false), is_reg(false) { }
  bool test() const {
    if (is_addr) {
      if (read) return rob().test_read(start, end);
      else return rob().test_write(start, end);
    }
    if (is_reg) {
      if (read) return rob().test_read(reg);
      else return rob().test_write(reg);
    }
    return true;
  }
  void lock(future_t::ptr fut) {
    //std::cout << "controller lock " << fut << std::endl;
    //std::cout << fut->finished << std::endl;
    if (is_addr) {
      if (read) rob().lock_read(fut, start, end);
      else rob().lock_write(fut, start, end);
    }
    if (is_reg) {
      if (read) rob().lock_read(fut, reg);
      else rob().lock_write(fut, reg);
    }
  }
};

struct controller_t : public coroutine_t {
  bool halted = false, in_except = false, stuck = false;
  int64_t pc = 0;
  int64_t pce = -1;
  std::vector<inst_t> mq;
  std::vector<inst_t> exq;
  std::list<int64_t> ex_entry;

  inst_t i;
  int64_t spmad, size, size2, size3, xo, yo;
  std::vector<rob_test_t> tests;
  cell_t dcell, lcell, rcell;
  int tno;
  future_t::ptr h;
  cell_t ock, ck;

#define try_issue_to(comp) do { if (!std::all_of(tests.begin(), tests.end(), [](auto&& p){ return p.test(); })) lbthrow(0); if (vq().full()) lbthrow(0); h = vq().issue(i); for (auto&& t : tests) t.lock(h); yield(ellapse(1. / frequency)); } while(0)
#define lbthrow(t) do { tno = (t); goto lbcatch; } while(0)

  async(while(!halted){
    yield(posedge(global_time));
    /*ck = cache().data_array.ref((int64_t)449224);
    if (ck != ock) {
      char c;
      log("checkpoint ptr/#449224 " << ock.data.f << "->" << ck.data.f);
      std::cin >> c;
      ock = ck;
    }*/
    lbtry: { if (pc >= mq.size()) halted = true; else {
      log("fetch inst from pc = " << pc);
      i = mq[pc++];
      log("pc = " << pc << ", name: " << i.op);
      tests.clear();
      gr().ref(0) = dcell = lcell = rcell = cell_t();
      for (auto p = i.args.begin(); p != i.args.end(); p++) {
        if (*p is typeid(reg_t) && p != i.args.begin()) {
          auto regid = (*p)->eval();
          if (regid == 0) {
            *p = std::make_shared<iimm_t>(0);
          } else {
            if (!rob().test_read(reg_t(regid))) lbthrow(0);
          }
        } else if (*p is typeid(raddr_t)) {
          int64_t regid = (*p)->eval();
          log("arg " << std::distance(i.args.begin(), p) << " raddr regid = " << regid);
          if (regid == 0) {
            *p = std::make_shared<iimm_t>(gr().ref(regid).data.i);
          } else {
            if (!rob().test_read(reg_t(regid))) lbthrow(0);
            if (!gr().ref(regid).det) {
              stat().udet_mem++;
              gr().ref(regid).data.i = 1048576;
            }
            *p = std::make_shared<iimm_t>(gr().ref(regid).data.i);
            log("arg " << std::distance(i.args.begin(), p) << " raddr regid = " << regid
               << ", data = " << gr().ref(regid).data.i);
          }
        }
      }
      if (i.op == "strideio"s) {
        spmad = i.args[i.args[0]->eval() < spm_size ? 1 : 0]->eval();
        size = i.args[2]->eval() * i.args[4]->eval();
        tests.emplace_back(i.args[0]->eval() >= spm_size, spmad, spmad + size);
        try_issue_to(vq);
      } else if (i.op == "loadv"s) {
        spmad = i.args[0]->eval();
        size = i.args[2]->eval();
        tests.emplace_back(false, spmad, spmad + size);
        try_issue_to(vq);
      } else if (i.op == "storev"s) {
        spmad = i.args[1]->eval();
        size = i.args[2]->eval();
        tests.emplace_back(true, spmad, spmad + size);
        try_issue_to(vq);
      } else if (i.op in cc("movv"s, "act"s, "floor"s, "mulvf"s, "divvf"s, "addvf"s, "subvf"s, "subfv"s, "ltvf"s,
                            "gtvf"s, "levf"s, "gevf"s, "eqvf"s, "nevf"s)) {
        size = i.args.back()->eval();
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size);
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size);
        try_issue_to(vq);
      } else if (i.op in cc("mulv"s, "divv"s, "addv"s, "subv"s, "ltv"s, "lev"s, "gev"s, "eqv"s, "nev"s)) {
        size = i.args.back()->eval();
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size);
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size);
        tests.emplace_back(true, i.args[2]->eval(), i.args[2]->eval() + size);
        try_issue_to(vq);
      } else if (i.op in cc("haddv"s, "hmulv"s, "hminv"s, "hmaxv"s)) {
        size = i.args.back()->eval();
        tests.emplace_back(false, reg_t(i.args[0]->eval()));
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size);
        try_issue_to(vq);
      } else if (i.op in cc("pool"s)) {
        size = i.args[2]->eval() * i.args[7]->eval() * i.args[8]->eval() * i.args[9]->eval();
        xo = (i.args[7]->eval() - i.args[3]->eval() + i.args[10]->eval() * 2 + i.args[5]->eval()) / i.args[5]->eval();
        yo = (i.args[8]->eval() - i.args[4]->eval() + i.args[11]->eval() * 2 + i.args[6]->eval()) / i.args[6]->eval();
        size2 = i.args[2]->eval() * xo * yo * i.args[9]->eval();
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size2 * 2);
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size * 2);
        try_issue_to(vq);
      } else if (i.op in cc("conv"s)) {
        size = i.args[3]->eval() * i.args[7]->eval() * i.args[8]->eval() * i.args[9]->eval();
        xo = (i.args[7]->eval() - i.args[5]->eval() + i.args[12]->eval() * 2 + i.args[10]->eval()) / i.args[10]->eval();
        yo = (i.args[8]->eval() - i.args[6]->eval() + i.args[13]->eval() * 2 + i.args[11]->eval()) / i.args[11]->eval();
        size2 = i.args[4]->eval() * xo * yo * i.args[9]->eval();
        size3 = i.args[4]->eval() * i.args[5]->eval() * i.args[6]->eval() * i.args[3]->eval();
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size2 * 2);
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size3 * 2);
        tests.emplace_back(true, i.args[2]->eval(), i.args[2]->eval() + size * 2);
        try_issue_to(vq);
      } else if (i.op in cc("mm"s)) {
        size = i.args[4]->eval() * i.args[6]->eval() * i.args[7]->eval();
        size2 = i.args[5]->eval() * i.args[6]->eval() * i.args[7]->eval();
        size3 = i.args[4]->eval() * i.args[5]->eval() * i.args[7]->eval();
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size2 * 2);
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size3 * 2);
        tests.emplace_back(true, i.args[2]->eval(), i.args[2]->eval() + size * 2);
        try_issue_to(vq);
      } else if (i.op in cc("trans"s)) {
        size = i.args[2]->eval() * i.args[3]->eval();
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size * 2);
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size * 2);
        try_issue_to(vq);
      } else if (i.op in cc("cycleadd"s)) {
        size = i.args[3]->eval();
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size);
        tests.emplace_back(true, i.args[1]->eval(), i.args[1]->eval() + size);
        tests.emplace_back(true, i.args[2]->eval(), i.args[2]->eval() + i.args[4]->eval());
        try_issue_to(vq);
      } else if (i.op in cc("movsv"s)) {
        size = line_bytes;
        tests.emplace_back(false, i.args[0]->eval(), i.args[0]->eval() + size);
        try_issue_to(vq);
      } else {
        if (i.op in cc("nop"s)) {
        } else if (i.op in cc("halt"s)) {
          halted = true;
        } else if (i.op in cc("printcr"s)) {
          std::cout << std::endl;
        } else if (i.op in cc("print"s)) {
          if (i.args[0] is typeid(output_t))
            std::cout << std::dynamic_pointer_cast<output_t>(i.args[0])->content;
          else if (i.args[0] is typeid(reg_t))
            std::cout << gr().ref(i.args[0]->eval()).data.i;
          else
            std::cout << cache().data_array.ref(i.args[0]->eval()).data.i;
        } else if (i.op in cc("printf"s)) {
          if (i.args[0] is typeid(output_t))
            std::cout << std::dynamic_pointer_cast<output_t>(i.args[0])->content;
          else if (i.args[0] is typeid(reg_t))
            std::cout << gr().ref(i.args[0]->eval()).data.f;
          else
            std::cout << cache().data_array.ref(i.args[0]->eval()).data.f;
        } else if (i.op in cc("jz"s)) {
          dcell.data.i = pc;
          lcell = gr().ref(i.args[0]->eval());
          if (!gr().ref(i.args[0]->eval()).det) {
            stat().udet_jmp++;
            lcell.det = true;
            lcell.data.i = ((int)(global_time / frequency) * 6364136223846793005ULL + 1442695040888963407ULL) & 1;
          }
          rcell.data.i = i.args[1]->eval();
          await(spu().issue(i.op, &dcell, lcell, rcell));
          pc = dcell.data.i;
        } else if (i.op in cc("yield"s)) {
          yield(ellapse(1. / frequency));
          gr().ref(i.args[0]->eval()).data.i++;
          lbthrow(1);
        } else if (i.op in cc("await"s)) {
          yield(ellapse(1. / frequency));
          if (gr().ref(i.args[0]->eval()).data.i) {
            gr().ref(i.args[0]->eval()).data.i--;
          } else lbthrow(0);
        } else if (i.op in cc("raise"s)) {
          yield(ellapse(1. / frequency));
          ex_entry.push_back(i.args[0]->eval());
        } else if (i.op in cc("movvs"s)) {
          spmad = i.args[1]->eval();
          tests.emplace_back(true, spmad, spmad + line_bytes);
          if (!std::all_of(tests.begin(), tests.end(), [](auto&& p){ return p.test(); })) lbthrow(0);
          await(spm(0).get(spmad, &dcell));
          gr().ref(i.args[0]->eval()).det = false;
        } else if (i.op in cc("movfs"s)) {
          yield(ellapse(1. / frequency));
          gr().ref(i.args[0]->eval()).data.f = std::dynamic_pointer_cast<fimm_t>(i.args[1])->imm;
          gr().ref(i.args[0]->eval()).det = true;
        } else if (i.op in cc("movs"s)) {
          yield(ellapse(1. / frequency));
          gr().ref(i.args[0]->eval()) = gr().ref(i.args[1]->eval());
        } else if (i.op in cc("loads"s)) {
          log("load data from address " << i.args[1]->eval() << " to reg " << i.args[0]->eval());
          await(cache().get(i.args[1]->eval(), &gr().ref(i.args[0]->eval())));
          log("value after load: " << gr().ref(i.args[0]->eval()).data.i);
        } else if (i.op in cc("stores"s)) {
          log("store data from reg " << i.args[1]->eval() << " to address " << i.args[0]->eval());
          log("value from reg: " << gr().ref(i.args[1]->eval()).data.i);
          await(cache().set(i.args[0]->eval(), &gr().ref(i.args[1]->eval())));
        } else if (i.op in cc("cvtif"s, "cvtfi"s, "noti"s, "movis"s)) {
          if (i.args[1] is typeid(iimm_t)) lcell.data.i = i.args[1]->eval();
          else lcell = gr().ref(i.args[1]->eval());
          await(spu().issue(i.op, &dcell, lcell, rcell));
          log("unary scalar inst write " << dcell.data.i << " to reg " << i.args[0]->eval());
          gr().ref(i.args[0]->eval()) = dcell;
          log("value after write: " << gr().ref(i.args[0]->eval()).data.i);
        } else {
          if (i.args[1] is typeid(iimm_t)) lcell.data.i = i.args[1]->eval();
          else lcell = gr().ref(i.args[1]->eval());
          if (i.args[2] is typeid(iimm_t)) rcell.data.i = i.args[2]->eval();
          else { rcell = gr().ref(i.args[2]->eval());
          log("read rhs from reg " << i.args[2]->eval());
          log("reg value: " << gr().ref(i.args[2]->eval()).data.i << ", rcell value: " << rcell.data.i);
          }
          await(spu().issue(i.op, &dcell, lcell, rcell));
          log("spu exec " << i.op << " finish");
          gr().ref(i.args[0]->eval()) = dcell;
          log("reg " << i.args[0]->eval() << " value after exec: " << gr().ref(i.args[0]->eval()).data.i);
        }
      }
      stuck = false;
    } } continue; lbcatch: {
      if (tno) {
        log(__LINE__);
        pc = -1;
        std::swap(pc, pce);
        std::swap(mq, exq);
        in_except = false;
        continue;
      } else {
        pc--;
        if (stuck == true) {
          log(__LINE__);

        } else if (in_except || pce >= 0 || !ex_entry.empty()) {
          log(__LINE__);
          std::swap(pc, pce);
          std::swap(mq, exq);
          in_except = !in_except;
          stuck = true;
          if (pc < 0) {
            log(__LINE__);
            pc = ex_entry.front();
            ex_entry.pop_front();
          }
          continue;
        }
      }
    }
    log(__LINE__ << " " << rob().fin << ":" << rob().fin->finished);
    await(rob().fin);
    log(__LINE__);
    stuck = false;
  })
};

void readraw(std::vector<inst_t>& main_inst, std::vector<inst_t>& except_inst, const char * filename) {
  std::ifstream sf(filename);
  char c;
  std::string line;
  while((c=sf.get()) != EOF) {
    if (c == '\n') {
      if (line == "main:"s) {

      } else if (line == "except:") {
        std::swap(main_inst, except_inst);
      } else {
        std::istringstream ss(line);
        inst_t i;
        ss >> i.pc;
        while((c=ss.get())==' ');
        do { i.op.push_back(c); } while(!((c=ss.get()) in cc(' ', EOF)));
        while ((c=ss.get()) != EOF) {
          if (c in cc('#') && i.op == "movfs"s) {
            double v;
            ss >> v;
            i.args.push_back(std::make_shared<fimm_t>(v));
          } else if (c in cc('#', '!')) {
            int64_t v;
            ss >> v;
            i.args.push_back(std::make_shared<iimm_t>(v));
          } else if (c == 'p') {
            ss.get(); // t
            ss.get(); // r
            ss.get(); // /
            c = ss.get();
            int64_t v;
            ss >> v;
            if (c == 'r') i.args.push_back(std::make_shared<raddr_t>(reg_t(v)));
            if (c == '#') i.args.push_back(std::make_shared<iaddr_t>(v));
          } else if (c == 'r') {
            int64_t v;
            ss >> v;
            i.args.push_back(std::make_shared<reg_t>(v));
          } else {
            std::string num, str;
            do { num.push_back(c); c=ss.get(); } while(c>='0' && c<='9');
            for (int count = 0; count < std::atoi(num.c_str()); count++) str.push_back(ss.get());
            i.args.push_back(std::make_shared<output_t>(str));
          }
          while(!(ss.get() in cc(' ', EOF)));
        }
        except_inst.push_back(i);
      }
      line.clear();
    } else {
      line.push_back(c);
    }
  }
}

int main(int argc, char** argv) {
  controller_t controller;
  readraw(controller.mq, controller.exq, (argc > 1 ? argv[1] : "eop.out"));
  controller.exq.insert(controller.exq.begin(), controller.mq.begin(), controller.mq.end());
  for (auto&& p : coroutine_t::list())
    p->operator()();
  while(!eq.empty()) {
    auto e = eq.top(); eq.pop();
    if (global_time <= e.time) {
      stat().stationary(ddr().sleep, ppu().sleep, spu().sleep, global_time, e.time);
      global_time = e.time;
      e.callback->operator()();
    }
  }
  std::cout << "[" << global_time << "] " << "processor halted: " << controller.halted << "." << std::endl;
  stat().report(std::cout);
  std::ofstream of(argc > 2 ? argv[2] : "eopsim.report");
  stat().report(of);
  return 0;
}
