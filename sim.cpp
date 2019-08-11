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
#include <typeinfo>
#include <typeindex>
#include <cmath>

// #include "data_loader.h"

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

constexpr int64_t spm_size = 1024 * 1024;
constexpr int64_t vq_depth = 8;
// ================================================================ CPULESS CHARACTERISTICS =====

using timestamp_t = double;
timestamp_t global_time = 0;
#define ellapse(t) (global_time+(t))
#define posedge(t) [](timestamp_t _t){return ellapse(((_t)*frequency-std::floor((_t)*frequency))/frequency);}(t)

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
  std::vector<coroutine_t*> _ftcb;
#define await(fut) do{if([&](auto&& _fut){if(!_fut||_fut->finished)return false;_fut->_ftcb.push_back(this);_crbp=__LINE__;return true;}(fut))return;case __LINE__:;}while(0)
#define finish(fut) do{for(auto&&p:(fut)->_ftcb)eq.emplace(ellapse(0),p);fut->finished=true;}while(0)
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
    return (req.size + line_bytes - 1) / line_bytes / frequency / 2.;
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

struct param_t {
  using ptr = std::shared_ptr<param_t>;
  virtual int64_t eval() const = 0;
};

struct reg_t : public param_t {
  int64_t regid;
  virtual int64_t eval() const { return regid; }
  reg_t(int64_t regid) : regid(regid) { }
  reg_t() : regid(0) { }
};

struct imm_t : public param_t {
  int64_t imm;
  virtual int64_t eval() const { return imm; }
};

struct addr_t : public param_t {

};

struct raddr_t : public addr_t {
  reg_t reg;
  virtual int64_t eval() const { return gr().ref(reg.regid).data.i; }
};

struct iaddr_t : public addr_t {
  imm_t imm;
  virtual int64_t eval() const { return imm.eval(); }
};

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
  bool test_read(reg_t r) const {
    return std::all_of(gr_lock.lower_bound(r.regid), gr_lock.upper_bound(r.regid), [](auto&& p){return p.second.read;});
  }
  bool test_write(reg_t r) const {
    return std::none_of(gr_lock.lower_bound(r.regid), gr_lock.upper_bound(r.regid), [](auto&& p){return true;});
  }
  int64_t test_read(int64_t start, int64_t end) const {
    std::multimap<int64_t, lock_t> deps;
    std::set_intersection(
        spm_begins.begin(), spm_begins.upper_bound(end),
        spm_ends.lower_bound(start), spm_ends.end(),
        std::inserter(deps, deps.end()), [](auto&& p1, auto&& p2){ return p1.second < p2.second; }
    );
    return std::all_of(deps.begin(), deps.end(), [](auto&& p){return p.second.read;});
  }
  int64_t test_write(int64_t start, int64_t end) const {
    std::multimap<int64_t, lock_t> deps;
    std::set_intersection(
        spm_begins.begin(), spm_begins.upper_bound(end),
        spm_ends.lower_bound(start), spm_ends.end(),
        std::inserter(deps, deps.end()), [](auto&& p1, auto&& p2){ return p1.second < p2.second; }
    );
    return deps.empty();
  }
  void unlock(int64_t lock_seq) {
    if (lock_seq > 0) {
      for (auto i = gr_lock.begin(); i != gr_lock.end(); i++) {
        if (i->second.seq == lock_seq) { gr_lock.erase(i); return; }
      }
    } else {
      for (auto i = spm_begins.begin(); i != spm_begins.end(); i++) {
        if (i->second.seq == lock_seq) { spm_begins.erase(i); break; }
      }
      for (auto i = spm_ends.begin(); i != spm_ends.end(); i++) {
        if (i->second.seq == lock_seq) { spm_ends.erase(i); return; }
      }
    }
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
      unlock(reqs.front().lock_seq);
      reqs.pop_front();
      finish(fin);
      fin = future_t::new_();
    }
    hibernate;
  })
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
    spm_begins.emplace(start, lock_t(rob_seq, true));
    spm_ends.emplace(end, lock_t(rob_seq, true));
    reqs.emplace_back(-rob_seq, fut);
    rob_seq++; awake;
  }
  void lock_write(future_t::ptr fut, int64_t start, int64_t end) {
    spm_begins.emplace(start, lock_t(rob_seq, false));
    spm_ends.emplace(end, lock_t(rob_seq, false));
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
      { "ltf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.f < r.data.f, 1; } },
      { "gti"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i > r.data.i, 1; } },
      { "gtf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.f > r.data.f, 1; } },
      { "lei"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i <= r.data.i, 1; } },
      { "lef"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.f <= r.data.f, 1; } },
      { "gei"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i >= r.data.i, 1; } },
      { "gef"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.f >= r.data.f, 1; } },
      { "eqi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i == r.data.i, 1; } },
      { "eqf"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.f == r.data.f, 1; } },
      { "nei"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i != r.data.i, 1; } },
      { "nef"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.f != r.data.f, 1; } },
      { "andi"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i & r.data.i, 1; } },
      { "xori"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i ^ r.data.i, 1; } },
      { "ori"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i | r.data.i, 1; } },
      { "noti"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = ~l.data.i, 1; } },
      { "movis"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i, 1; } },
      { "movfs"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i, 1; } },
      { "jz"s, +[](cell_t* d, cell_t l, cell_t r){ return d->data.i = l.data.i ? d->data.i : r.data.i, 1; } },
    };
    return ops[req.op](req.dest, req.lhs, req.rhs);
  }
  std::list<spu_request_t> reqs;
  async(
    while(1) {
      while(!reqs.empty()) {
        yield(posedge(global_time));
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
  cell_t cell;
  async(
    while(1) {
      while(!reqs.empty() && mode != NONE) {
        yield(posedge(global_time));
        // pipeline 3: wb
        if (mode == NONE) {
        } else if (mode in std::set<int>{REDUCE}) {
        } else if (mode in std::set<int>{UNARY, BINARY, POOL, CONV, MM, MOVSV}) {
          if (cycles < total_cycles - 1)
            hs.push_back(spm(0).set(0, &cell));
        }
        // pipeline 2: ex
        hs.push_back(future_t::new_());
        finish_at(hs.back(), ellapse(0.5/frequency));
        // pipeline 1: ld
        if (mode == NONE) {
        } else if (mode in std::set<int>{UNARY, REDUCE}) {
          if (cycles > 1)
            hs.push_back(spm(0).get(0, &cell));
        } else if (mode in std::set<int>{BINARY, POOL, CONV, MM}) {
          if (cycles > 1) {
            hs.push_back(spm(0).get(0, &cell));
            hs.push_back(spm(0).get(1, &cell));
          }
        }
        // pipeline 0: ctrl
        if (mode == NONE) {
          if (reqs.front().op in cc("movsv"s)) {
            mode = MOVSV;
            cycles = 1;
            total_cycles = 3;
          } else if (reqs.front().op in cc("movv"s, "act"s, "floor"s, "mulvf"s, "divvf"s, "addvf"s, "subvf"s, "subfv"s,
                               "ltvf"s, "gtvf"s, "levf"s, "gevf"s, "eqvf"s, "nevf"s)) {
            mode = UNARY;
            cycles = total_cycles = (reqs.front().size[0] + line_bytes - 1) / line_bytes + 2;
          } else if (reqs.front().op in cc("mulv"s, "divv"s, "addv"s, "subv"s,
                               "ltv"s, "gtv"s, "lev"s, "gev"s, "eqv"s, "nev"s)) {
            mode = BINARY;
            cycles = total_cycles = (reqs.front().size[0] + line_bytes - 1) / line_bytes + 2;
          } else if (reqs.front().op in cc("haddv"s, "hmulv"s, "hminv"s, "hmaxv"s)) {
            mode = REDUCE;
            cycles = total_cycles = (reqs.front().size[0] + line_bytes - 1) / line_bytes + 2;
          } else if (reqs.front().op in cc("pool"s)) {
            mode = POOL;
            int64_t fi = (reqs.front().size[0] * 2 + line_bytes - 1) / line_bytes;
            int64_t kx = reqs.front().size[1];
            int64_t ky = reqs.front().size[2];
            int64_t sx = reqs.front().size[3];
            int64_t sy = reqs.front().size[4];
            int64_t xi = reqs.front().size[5];
            int64_t yi = reqs.front().size[6];
            int64_t bt = reqs.front().size[7];
            int64_t px = reqs.front().size[8];
            int64_t py = reqs.front().size[9];
            int64_t xo = (xi - kx + px * 2 + sx) / sx;
            int64_t yo = (yi - ky + py * 2 + sy) / sy;
            cycles = total_cycles = bt * xo * yo * fi * kx * ky + 2;
          } else if (reqs.front().op in cc("conv"s)) {
            mode = CONV;
            int64_t fi = (reqs.front().size[0] * 2 + line_bytes - 1) / line_bytes;
            int64_t fo = (reqs.front().size[1] * 2 + line_bytes - 1) / line_bytes;
            int64_t kx = reqs.front().size[2];
            int64_t ky = reqs.front().size[3];
            int64_t xi = reqs.front().size[4];
            int64_t yi = reqs.front().size[5];
            int64_t bt = reqs.front().size[6];
            int64_t sx = reqs.front().size[7];
            int64_t sy = reqs.front().size[8];
            int64_t px = reqs.front().size[9];
            int64_t py = reqs.front().size[10];
            int64_t xo = (xi - kx + px * 2 + sx) / sx;
            int64_t yo = (yi - ky + py * 2 + sy) / sy;
            cycles = total_cycles = bt * xo * yo * fi * fo * kx * ky + 2;
          } else if (reqs.front().op in cc("mm"s)) {
            mode = MM;
            int64_t fi = (reqs.front().size[0] * 2 + line_bytes - 1) / line_bytes;
            int64_t fo = (reqs.front().size[1] * 2 + line_bytes - 1) / line_bytes;
            int64_t ni = reqs.front().size[2];
            int64_t bt = reqs.front().size[3];
            cycles = total_cycles = bt * ni * fi * fo + 2;
          } else if (reqs.front().op in cc("trans"s)) {
            mode = UNARY;
            int64_t n1 = (reqs.front().size[0] * 2 + line_bytes - 1) / line_bytes;
            int64_t n2 = (reqs.front().size[1] * 2 + line_bytes - 1) / line_bytes;
            cycles = total_cycles = n1 * n2 + 2;
          }
        } else {
          if (!(cycles-->0)) {
            mode = NONE;
            finish(reqs.front().fut);
            reqs.pop_front();
            cycles = total_cycles = 0;
          }
        }
        while(!hs.empty()) {
          await(hs.front());
          hs.pop_front();
        }
      }
      hibernate;
    }
  )
  future_t::ptr issue(std::string op, std::vector<int64_t> args) {
    future_t::ptr fut = future_t::new_();
    reqs.emplace_back(op, args, fut); awake;
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
        hs.push_back(ddr().set(reqs.front().addr, reqs.front().size));
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
      for (int i = 0; i < args[4]->eval(); i++) {
        reqs.emplace_back(args[load ? 0 : 1]->eval() + i * args[3]->eval(), args[2]->eval(), load,
                          i == args[4]->eval() - 1 ? fut : nullptr);
      }
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
  std::list<inst_t> q;
  future_t::ptr fin = future_t::new_();

#define vqii(...) do { \
          while (!rob().test_write(__VA_ARGS__)) \
            await(rob().fin); \
          h = pdma().issue(i.op, i.args); \
          rob().lock_write(h, __VA_ARGS__);\
        } while(0)

#define vqio(...) do { \
          while (!rob().test_read(__VA_ARGS__)) \
            await(rob().fin); \
          h = pdma().issue(i.op, i.args); \
          rob().lock_read(h, __VA_ARGS__);\
        } while(0)

#define vqiu(startw, endw, startr, endr) do { \
          while (!rob().test_write(startw, endw) || !rob().test_read(startr, endr)) \
            await(rob().fin); \
          h = ppu().issue(i.op, std::vector<int64_t>{size}); \
          rob().lock_write(h, startw, endw); rob().lock_read(h, startr, endr);\
        } while(0)

#define vqib(startw, endw, startl, endl, startr, endr) do { \
          while (!rob().test_write(startw, endw) || !rob().test_read(startl, endl) || !rob().test_read(startr, endr)) \
            await(rob().fin); \
          h = ppu().issue(i.op, std::vector<int64_t>{size}); \
          rob().lock_write(h, startw, endw); rob().lock_read(h, startl, endl); rob().lock_read(h, startr, endr);\
        } while(0)

#define vqih(reg, startr, endr) do { \
          while (!rob().test_read(startr, endr)) \
            await(rob().fin); \
          h = ppu().issue(i.op, std::vector<int64_t>{size}); \
          rob().lock_write(h, reg); rob().lock_read(h, startr, endr);\
        } while(0)

  bool load;
  int64_t spmad, size, size2, size3, xo, yo;
  inst_t i;
  future_t::ptr h;
  std::vector<int64_t> sizes;
  async(while(1){                 
    while(!q.empty()) {           
      i = q.front();              
      if (i.op == "strideio"s) {
        load = i.args[0]->eval() < spm_size;
        spmad = i.args[load ? 1 : 0]->eval();
        size = i.args[2]->eval() * i.args[4]->eval();
        if (load) vqii(spmad, spmad + size);
        else vqio(spmad, spmad + size);
      } else if (i.op == "loadv"s) {
        spmad = i.args[0]->eval();
        size = i.args[2]->eval();
        vqii(spmad, spmad + size);
      } else if (i.op == "storev"s) {
        spmad = i.args[1]->eval();
        size = i.args[2]->eval();
        vqio(spmad, spmad + size);
      } else if (i.op in cc("movv"s, "act"s, "floor"s, "mulvf"s, "divvf"s, "addvf"s, "subvf"s, "subfv"s, "ltvf"s,
                            "gtvf"s, "levf"s, "gevf"s, "eqvf"s, "nevf"s)) {
        size = i.args.back()->eval();
        vqiu(i.args[0]->eval(), i.args[0]->eval() + size, i.args[1]->eval(), i.args[1]->eval() + size);
      } else if (i.op in cc("mulv"s, "divv"s, "addv"s, "subv"s, "ltv"s, "lev"s, "gev"s, "eqv"s, "nev"s)) {
        size = i.args.back()->eval();
        vqib(i.args[0]->eval(), i.args[0]->eval() + size,
             i.args[1]->eval(), i.args[1]->eval() + size,
             i.args[2]->eval(), i.args[2]->eval() + size);
      } else if (i.op in cc("haddv"s, "hmulv"s, "hminv"s, "hmaxv"s)) {
        size = i.args.back()->eval();
        vqih(reg_t(i.args[0]->eval()), i.args[1]->eval(), i.args[1]->eval() + size);
      } else if (i.op in cc("pool"s)) {
        sizes.clear();
        for (int a = 0; a < 10; a++)
          sizes.push_back(i.args[2 + a]->eval());
        size = i.args[2]->eval() * i.args[7]->eval() * i.args[8]->eval() * i.args[9]->eval();
        xo = (i.args[7]->eval() - i.args[3]->eval() + i.args[10]->eval() * 2 + i.args[5]->eval()) / i.args[5]->eval();
        yo = (i.args[8]->eval() - i.args[4]->eval() + i.args[11]->eval() * 2 + i.args[6]->eval()) / i.args[6]->eval();
        size2 = i.args[2]->eval() * xo * yo * i.args[9]->eval();
        while (!rob().test_write(i.args[0]->eval(), i.args[0]->eval() + size2 * 2)
            || !rob().test_read(i.args[1]->eval(), i.args[1]->eval() + size * 2))
          await(rob().fin);
        h = ppu().issue(i.op, sizes);
        rob().lock_write(h, i.args[0]->eval(), i.args[0]->eval() + size2 * 2);
        rob().lock_read(h, i.args[1]->eval(), i.args[1]->eval() + size * 2);
      } else if (i.op in cc("conv"s)) {
        sizes.clear();
        for (int a = 0; a < 11; a++)
          sizes.push_back(i.args[3 + a]->eval());
        size = i.args[3]->eval() * i.args[7]->eval() * i.args[8]->eval() * i.args[9]->eval();
        xo = (i.args[7]->eval() - i.args[5]->eval() + i.args[12]->eval() * 2 + i.args[10]->eval()) / i.args[10]->eval();
        yo = (i.args[8]->eval() - i.args[6]->eval() + i.args[13]->eval() * 2 + i.args[11]->eval()) / i.args[11]->eval();
        size2 = i.args[4]->eval() * xo * yo * i.args[9]->eval();
        size3 = i.args[4]->eval() * i.args[5]->eval() * i.args[6]->eval() * i.args[3]->eval();
        while (!rob().test_write(i.args[0]->eval(), i.args[0]->eval() + size2 * 2)
            || !rob().test_read(i.args[1]->eval(), i.args[1]->eval() + size3 * 2)
            || !rob().test_read(i.args[2]->eval(), i.args[2]->eval() + size * 2))
          await(rob().fin);
        h = ppu().issue(i.op, sizes);
        rob().lock_write(h, i.args[0]->eval(), i.args[0]->eval() + size2 * 2);
        rob().lock_read(h, i.args[1]->eval(), i.args[1]->eval() + size3 * 2);
        rob().lock_read(h, i.args[2]->eval(), i.args[2]->eval() + size * 2);
      } else if (i.op in cc("mm"s)) {
        sizes.clear();
        for (int a = 0; a < 4; a++)
          sizes.push_back(i.args[3 + a]->eval());
        size = i.args[3]->eval() * i.args[5]->eval() * i.args[6]->eval();
        size2 = i.args[4]->eval() * i.args[5]->eval() * i.args[6]->eval();
        size3 = i.args[3]->eval() * i.args[4]->eval() * i.args[6]->eval();
        while (!rob().test_write(i.args[0]->eval(), i.args[0]->eval() + size2 * 2)
            || !rob().test_read(i.args[1]->eval(), i.args[1]->eval() + size3 * 2)
            || !rob().test_read(i.args[2]->eval(), i.args[2]->eval() + size * 2))
          await(rob().fin);
        h = ppu().issue(i.op, sizes);
        rob().lock_write(h, i.args[0]->eval(), i.args[0]->eval() + size2 * 2);
        rob().lock_read(h, i.args[1]->eval(), i.args[1]->eval() + size3 * 2);
        rob().lock_read(h, i.args[2]->eval(), i.args[2]->eval() + size * 2);
      } else if (i.op in cc("trans"s)) {
        sizes.clear();
        for (int a = 0; a < 2; a++)
          sizes.push_back(i.args[2 + a]->eval());
        size = i.args[2]->eval() * i.args[3]->eval();
        while (!rob().test_write(i.args[0]->eval(), i.args[0]->eval() + size * 2)
            || !rob().test_read(i.args[1]->eval(), i.args[1]->eval() + size * 2))
          await(rob().fin);
        h = ppu().issue(i.op, sizes);
        rob().lock_write(h, i.args[0]->eval(), i.args[0]->eval() + size * 2);
        rob().lock_read(h, i.args[1]->eval(), i.args[1]->eval() + size * 2);
      } else if (i.op in cc("cycleadd"s)) {
        size = i.args[3]->eval();
        vqib(i.args[0]->eval(), i.args[0]->eval() + size,
             i.args[1]->eval(), i.args[1]->eval() + size,
             i.args[2]->eval(), i.args[2]->eval() + i.args[4]->eval());
      } else if (i.op in cc("movsv"s)) {
        size = line_bytes;
        while (!rob().test_write(i.args[0]->eval(), i.args[0]->eval() + size))
          await(rob().fin);
        h = ppu().issue(i.op, std::vector<int64_t>{ size });
        rob().lock_write(h, i.args[0]->eval(), i.args[0]->eval() + size);
      }
      q.pop_front();
      finish(fin);
      fin = future_t::new_();
      yield(ellapse(1. / frequency));
    }
    hibernate;
  })
  void issue(inst_t i) {
    q.push_back(i);
    awake;
  }
  bool full() const {
    return q.size() >= vq_depth;
  }
};

vq_t& vq() {
  static vq_t _vq;
  return _vq;
}

struct controller_t : public coroutine_t {
  bool halted;
  int64_t pc;
  int64_t pce;
  std::vector<future_t::ptr> hs;
  async(while(!halted){

  })
};

bool is_number_in_readraw(char s[]) {
  // [0-9][0-9]*
  for (int i = 0; i < strlen(s); i++)
    if (s[i] > '9' or s[i] < '0')
      return false;
  return true;
}

int readraw(std::vector<inst_t> &main_inst, std::vector<inst_t> &except_inst, char filename[] = "eop.out") {
  FILE *fpin;
  if ((fpin=fopen(filename,"r")) == NULL) {
    std::cout << "[ERROR] No input src " << filename << std::endl;
    exit(-1);
  }
  char input_str[1024];
  int pc;
  std::string op;
  std::vector<inst_t> *now = &main_inst;

  while(fscanf(fpin, "%s", input_str) != EOF) {
    if (strcmp(input_str, "main:") == 0) {
      std::cout << "main inst" << std::endl;
      continue;
    }
    if (strcmp(input_str, "except:") == 0) {
      std::cout << "except inst" << std::endl;
      now = &except_inst;
      continue;
    }
    if (is_number_in_readraw(input_str)) {
      // PC
      int pc = atoi(input_str);
      std::string op;
      std::vector<param_t::ptr> args;
      (*now).push_back(inst_t(pc, op, args));
    } else if (input_str[0] == 'p' and input_str[1] == 't') {
      // ptr/#XXX or ptr/rXXX
      if (strlen(input_str) <= 5)
        std::cout << "ERROR in" << __LINE__ << input_str <<std::endl;
      char tmp_[1024];
      strcpy(tmp_, input_str + 5);
      int num_ = atoi(tmp_);
      if (input_str[4] == '#') {
        imm_t t_; std::shared_ptr<iaddr_t> addr_ = std::make_shared<iaddr_t>();
        t_.imm = num_; addr_->imm = t_;
        (*now).back().args.push_back(addr_);
      } else {
        reg_t t_; std::shared_ptr<raddr_t> addr_ = std::make_shared<raddr_t>();
        t_.regid = num_; addr_->reg = t_;
        (*now).back().args.push_back(addr_);
      }
    } else if (input_str[0] == '#' or input_str[0] == 'r' or input_str[0] == '!') {
      // #XXX or rXXX or !XXX
      char tmp_[1024];
      strcpy(tmp_, input_str + 5);
      int num_ = atoi(tmp_);
      if (input_str[0] == 'r') {
        std::shared_ptr<reg_t> t_ = std::make_shared<reg_t>();
        t_->regid = num_;
        (*now).back().args.push_back(t_);
      } else {
        std::shared_ptr<imm_t> t_ = std::make_shared<imm_t>();
        t_->imm = num_;
        (*now).back().args.push_back(t_);
      }
    } else if (atoi(input_str) > 0) {
      std::cout << "print!!!" << std::endl;
      ;
    } else {
      (*now).back().op = std::string(input_str);
    }
  }
  return 0;
}

int visualizing_in_checkinput(const std::vector<inst_t> inst) {
  for (size_t i = 0; i < inst.size(); i++) {
    printf("  %d ", inst[i].pc);
    std::cout << inst[i].op;
    // for (size_t j = 0; j < inst[i].args.size(); j++) {
    //   if inst[i].args[j]
    printf("\n");
  }
  return 0;
}

int checkinput(const std::vector<inst_t> main_inst, const std::vector<inst_t> except_inst) {
  printf("main:\n");
  visualizing_in_checkinput(main_inst);
  printf("except:\n");
  visualizing_in_checkinput(except_inst);
  return 0;
}

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

  std::vector<inst_t> main_inst;
  std::vector<inst_t> except_inst;
  readraw(main_inst, except_inst, "eop.out");
  printf("%d %d\n", main_inst.size(), except_inst.size());
  checkinput(main_inst, except_inst);

  return 0;
}
