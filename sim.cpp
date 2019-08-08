#include <variant>
#include <cstdint>
#include <vector>
#include <functional>
#include <memory>
#include <queue>
#include <list>
#include <map>
#include <iostream>
#include <typeinfo>

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

// ================================================================ CPULESS CHARACTERISTICS =====

using timestamp_t = double;
timestamp_t global_time = 0;
#define ellapse(t) (global_time+(t))

#define log(...) do{std::cout << "[" << global_time << "] " << __VA_ARGS__ << std::endl;}while(0)

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
  coroutine_t* _ftcb = nullptr;
#define await(fut) do{if([&](auto&& _fut){if(!_fut||_fut->finished)return false;_fut->_ftcb=this;_crbp=__LINE__;return true;}(fut))return;case __LINE__:;}while(0)
#define finish(fut) do{if((fut)->_ftcb)eq.emplace(ellapse(0),(fut)->_ftcb);fut->finished=true;}while(0)
#define finish_at(fut,ts) do{future_scheduler().schedule(ts,fut);}while(0)
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
  std::vector<cell_t> cells;
  int64_t word_size = 8;
  virtual cell_t& ref(int64_t addr) {
    addr /= word_size;
    if (addr >= cells.size()) cells.resize(addr + 1);
    return cells[addr];
  }
};

mem_t& gr() {
  static mem_t _gr;
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
      log("mem loop.");
      if (!get_finish && !gets.empty() && (duplex() || !set_finish) && (sets.empty() || gets.front().ts <= sets.front().ts)) {
        log("mem start get.");
        get_finish = ellapse(get_exec(gets.front()));
      } else if (!set_finish && !sets.empty() && (duplex() || !get_finish) && (gets.empty() || sets.front().ts <= gets.front().ts)) {
        log("mem start set.");
        set_finish = ellapse(set_exec(sets.front()));
      } else if (get_finish && (!set_finish || get_finish <= set_finish)) {
        log("mem exec get.");
        while (get_finish > global_time)
          yield(get_finish);
        get_finish = 0;
        if (gets.front().data) *gets.front().data = mem()->ref(gets.front().addr);
        log("mem get finish.");
        finish(gets.front().fut);
        gets.pop_front();
      } else if (set_finish && (!get_finish || get_finish > set_finish)) {
        log("mem exec set.");
        while (set_finish > global_time)
          yield(set_finish);
        set_finish = 0;
        if (sets.front().data)
          mem()->ref(sets.front().addr) = *sets.front().data;
        else
          mem()->ref(sets.front().addr).det = false;
        log("mem set finish.");
        finish(sets.front().fut);
        sets.pop_front();
      }
    }
    log("mem hibernate.");
    hibernate;
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
      gets.emplace_back(addr, gr().word_size, data, fut); awake;
      return fut;
    } else {
      *data = mem()->ref(addr);
      auto time = get_exec(get_request_t(addr, gr().word_size, data, fut));
      if (get_finish)
        get_finish += time;
      finish_at(fut, ellapse(time));
      return fut;
    }
  }
  future_t::ptr set(int64_t addr, cell_t* data) {
    future_t::ptr fut = future_t::new_();
    if (sleep) {
      sets.emplace_back(addr, gr().word_size, data, fut); awake;
      return fut;
    } else {
      mem()->ref(addr) = *data;
      auto time = set_exec(set_request_t(addr, gr().word_size, data, nullptr));
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
    return ((req.size + line_bytes - 1) / line_bytes) * line_bytes / ddr_bandwidth;
  }
  virtual timestamp_t set_exec(set_request_t req) {
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
    return (req.size + line_bytes - 1) / line_bytes / frequency;
  }
  virtual timestamp_t set_exec(set_request_t req) {
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

      log("cache lookup.");
      hit = lookup(req.addr);
      yield(ellapse(hit ? cache_hit_latency : cache_miss_latency));
      log("cache lookup finish.");

      if (req.mode == 0) { // read
        if (!hit) { // miss, must read from ddr.
          log("cache proxy ddr read.");
          await(ddr().get(req.addr, &data_array.ref(req.addr)));
          log("cache proxy ddr read finish.");
        }
        *req.data = data_array.ref(req.addr);
        finish(req.fut); // return result while writing cache.
        if (!hit) {
          log("cache record new tag.");
          record(req.addr);
          yield(ellapse(cache_write_latency));
          log("cache record new tag finish.");
        }
      } else { // write
        log("cache write non-blocking.");
        finish(req.fut); // execution dont need to wait cache writing.
        if (!hit || *req.data != data_array.ref(req.addr)) {
          log("cache proxy ddr write. cache write.");
          ddr().set(req.addr, req.data); // no await, write cache and ddr simutaneously.
          yield(ellapse(cache_write_latency));
          data_array.ref(req.addr) = *req.data;
          record(req.addr);
          log("cache write finish.");
        }
      }
    }
    log("cache hibernate.");
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

struct reg_t {
  int64_t regid;
};

struct addr_t {
  
};

struct raddr_t : public addr_t {
  reg_t reg;
};

struct iaddr_t : public addr_t {
  int64_t imm;
};

struct controller_t : public coroutine_t {
  int i;
  int j;
  bool halted;
  std::vector<future_t::ptr> hs;
  async(for(i = 0; i < 10; i++) {
    log(i);
    hs.push_back(ddr().get(10000 + i * 16, 1024));
    hs.push_back(ddr().set(10000 + i * 16 + 8, 1024));
    hs.push_back(cache().get(i * 16, new cell_t));
    hs.push_back(cache().set(i * 16 + 8, new cell_t));
    for (j = 0; j < 4; j++) await(hs[j]);
    hs.clear();
  } halted = true;)
};

int main() {
  controller_t controller;
  for (auto&& p : coroutine_t::list())
    p->operator()();
  while(!eq.empty()) {
    auto e = eq.top(); eq.pop();
    if (global_time <= e.time) {
      global_time = e.time;
      e.callback->operator()();
    }
  }
  return 0;
}
