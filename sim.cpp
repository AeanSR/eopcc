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
      if (!get_finish && !gets.empty() && (duplex() || !set_finish) && (sets.empty() || gets.front().ts <= sets.front().ts)) {
        get_finish = ellapse(get_exec(gets.front()));
      } else if (!set_finish && !sets.empty() && (duplex() || !get_finish) && (gets.empty() || sets.front().ts <= gets.front().ts)) {
        set_finish = ellapse(set_exec(sets.front()));
      } else if (get_finish && (!set_finish || get_finish <= set_finish)) {
        while (get_finish > global_time)
          yield(get_finish);
        get_finish = 0;
        if (gets.front().data) *gets.front().data = mem()->ref(gets.front().addr);
        finish(gets.front().fut);
        gets.pop_front();
      } else if (set_finish && (!get_finish || get_finish > set_finish)) {
        while (set_finish > global_time)
          yield(set_finish);
        set_finish = 0;
        if (sets.front().data)
          mem()->ref(sets.front().addr) = *sets.front().data;
        else
          mem()->ref(sets.front().addr).det = false;
        finish(sets.front().fut);
        sets.pop_front();
      }
    }
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

      hit = lookup(req.addr);
      yield(ellapse(hit ? cache_hit_latency : cache_miss_latency));

      if (req.mode == 0) { // read
        if (!hit) { // miss, must read from ddr.
          await(ddr().get(req.addr, &data_array.ref(req.addr)));
        }
        *req.data = data_array.ref(req.addr);
        finish(req.fut); // return result while writing cache.
        if (!hit) {
          record(req.addr);
          yield(ellapse(cache_write_latency));
        }
      } else { // write
        finish(req.fut); // execution dont need to wait cache writing.
        if (!hit || *req.data != data_array.ref(req.addr)) {
          ddr().set(req.addr, req.data); // no await, write cache and ddr simutaneously.
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
};

struct addr_t : public param_t {
  ;
};

struct raddr_t : public addr_t {
  reg_t reg;
  virtual int64_t eval() const { return gr().ref(reg.regid).data.i; }
  friend std::ostream &operator<<(std::ostream &out, const raddr_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
};

struct iaddr_t : public addr_t {
  iimm_t imm;
  virtual int64_t eval() const { return imm.eval(); }
  friend std::ostream &operator<<(std::ostream &out, const iaddr_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
};

struct output_t : public param_t {
  std::string content;
  // TODO(): print information
  virtual int64_t eval() const { return 0; }
  friend std::ostream &operator<<(std::ostream &out, const output_t &t);
  virtual int visualizing() const { std::cout << *this; return 0; }
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
    req.dest->det = req.lhs.det && req.rhs.det;
    return ops[req.op](req.dest, req.lhs, req.rhs);
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
  cell_t cell;
  async(
    while(1) {
      while(!reqs.empty() && mode != NONE) {
        yield(posedge(global_time));
        // pipeline 3: wb
        if (mode == NONE) {
        } else if (mode in std::set<int>{REDUCE}) {
        } else if (mode in std::set<int>{UNARY, BINARY, POOL, CONV, MM, MOVSV}) {
            hs.push_back(spm(0).set(0, &cell));
        }
        // pipeline 2: ex
        hs.push_back(future_t::new_());
        finish_at(hs.back(), ellapse(0.5/frequency));
        // pipeline 1: ld
        if (mode == NONE) {
        } else if (mode in std::set<int>{UNARY, REDUCE}) {
            hs.push_back(spm(0).get(0, &cell));
        } else if (mode in std::set<int>{BINARY, POOL, CONV, MM}) {
            hs.push_back(spm(0).get(0, &cell));
            hs.push_back(spm(0).get(1, &cell));
        }
        // pipeline 0: ctrl
        if (mode == NONE) {
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
            int64_t fi = (reqs.front().size[3] * 2 + line_bytes - 1) / line_bytes;
            int64_t fo = (reqs.front().size[4] * 2 + line_bytes - 1) / line_bytes;
            int64_t ni = reqs.front().size[5];
            int64_t bt = reqs.front().size[6];
            cycles = total_cycles = bt * ni * fi * fo;
          } else if (reqs.front().op in cc("trans"s)) {
            mode = UNARY;
            int64_t n1 = (reqs.front().size[2] * 2 + line_bytes - 1) / line_bytes;
            int64_t n2 = (reqs.front().size[3] * 2 + line_bytes - 1) / line_bytes;
            cycles = total_cycles = n1 * n2;
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
  std::list<std::pair<inst_t, future_t::ptr>> q;
  inst_t i;
  future_t::ptr h;
  std::vector<int64_t> sizes;
  async(while(1){                 
    while(!q.empty()) {
      i = q.front().first;
      if (i.op in cc("strideio"s, "loadv"s, "storev"s))
        pdma().issue(i.op, i.args);
      else
        ppu().issue(i.op, i.args);
      finish(q.front().second);
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

#define try_issue_to(comp) do { if (!std::all_of(tests.begin(), tests.end(), [](auto&& p){ return p.test(); })) lbthrow(0); if (vq().full()) lbthrow(0); h = comp().issue(i); for (auto&& t : tests) t.lock(h); yield(ellapse(1. / frequency)); } while(0)
#define lbthrow(t) do { tno = (t); goto lbcatch; } while(0)

  async(while(!halted){
    yield(posedge(global_time));
    log(__LINE__);
    lbtry: { if (pc >= mq.size()) halted = true; else {
      i = mq[pc++];
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
          if (regid == 0) {
            *p = std::make_shared<iimm_t>(gr().ref(regid).data.i);
          } else {
            if (!rob().test_read(reg_t(regid))) lbthrow(0);
            if (!gr().ref(regid).det) {
              std::cerr << "pc=" << pc << ", undetermined data referenced as address!" << std::endl;
            }
            *p = std::make_shared<iimm_t>(gr().ref(regid).data.i);
          }
        }
      }
    log(__LINE__);
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
        size = i.args[3]->eval() * i.args[5]->eval() * i.args[6]->eval();
        size2 = i.args[4]->eval() * i.args[5]->eval() * i.args[6]->eval();
        size3 = i.args[3]->eval() * i.args[4]->eval() * i.args[6]->eval();
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
        log(__LINE__ << i.op);
        if (i.op in cc("nop"s)) {
        } else if (i.op in cc("halt"s)) {
          halted = true;
        } else if (i.op in cc("print"s)) {
          log(__LINE__);
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
          if (!gr().ref(i.args[0]->eval()).det)
            std::cerr << "pc=" << pc << ", undetermined data referenced as condition!" << std::endl;
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
        } else if (i.op in cc("loads"s)) {
          await(cache().get(i.args[1]->eval(), &gr().ref(i.args[0]->eval())));
        } else if (i.op in cc("stores"s)) {
          await(cache().set(i.args[0]->eval(), &gr().ref(i.args[1]->eval())));
        } else if (i.op in cc("cvtif"s, "cvtfi"s, "noti"s, "movis"s)) {
          if (i.args[1] is typeid(iimm_t)) lcell.data.i = i.args[1]->eval();
          else lcell = gr().ref(i.args[1]->eval());
          await(spu().issue(i.op, &dcell, lcell, rcell));
          gr().ref(i.args[0]->eval()) = dcell;
        } else {
          if (i.args[1] is typeid(iimm_t)) lcell.data.i = i.args[1]->eval();
          else lcell = gr().ref(i.args[1]->eval());
          if (i.args[2] is typeid(iimm_t)) rcell.data.i = i.args[2]->eval();
          else rcell = gr().ref(i.args[2]->eval());
          await(spu().issue(i.op, &dcell, lcell, rcell));
          gr().ref(i.args[0]->eval()) = dcell;
        }
      }
      stuck = false;
    } } continue; lbcatch: {
      if (tno) {
        pc = -1;
        std::swap(pc, pce);
        std::swap(mq, exq);
        in_except = false;
        continue;
      } else {
        if (stuck == true) {

        } else if (in_except || pce >= 0 || !ex_entry.empty()) {
          pc--;
          std::swap(pc, pce);
          std::swap(mq, exq);
          in_except = !in_except;
          stuck = true;
          if (pc < 0) {
            pc = ex_entry.front();
            ex_entry.pop_front();
          }
          continue;
        }
      }
    }
    await(rob().fin);
    stuck = false;
  })
};

bool is_number_in_readraw(char s[]) {
  // [0-9][0-9]*
  for (int i = 0; i < strlen(s); i++)
    if ((s[i] > '9' or s[i] < '0') and s[i] != '-' and s[i] != '+')
      return false;
  return true;
}

int readraw(std::vector<inst_t> &main_inst, std::vector<inst_t> &except_inst, const char filename[]) {
  #define str2int64(t_) strtol(t_, NULL, 10)
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
      //std::cout << "main inst" << std::endl;
      continue;
    }
    if (strcmp(input_str, "except:") == 0) {
      //std::cout << "except inst" << std::endl;
      now = &except_inst;
      continue;
    }
    if (is_number_in_readraw(input_str)) {
      // PC
      int64_t pc = str2int64(input_str);
      std::string op;
      std::vector<param_t::ptr> args;
      (*now).push_back(inst_t(pc, op, args));
    } else if (input_str[0] == 'p' and input_str[1] == 't') {
      // ptr/#XXX or ptr/rXXX
      if (strlen(input_str) <= 5)
        std::cout << "ERROR in" << __LINE__ << input_str <<std::endl;
      char tmp_[1024];
      strcpy(tmp_, input_str + 5);
      int64_t num_ = str2int64(tmp_);
      if (input_str[4] == '#') {
        iimm_t t_; std::shared_ptr<iaddr_t> addr_ = std::make_shared<iaddr_t>();
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
      strcpy(tmp_, input_str + 1);
      if (input_str[0] == 'r') {
        int64_t num_ = str2int64(tmp_);
        std::shared_ptr<reg_t> t_ = std::make_shared<reg_t>();
        t_->regid = num_;
        (*now).back().args.push_back(t_);
      } else {
        if ((*now).back().op != std::string("movfs")) {
          int64_t num_ = str2int64(tmp_);
          std::shared_ptr<iimm_t> t_ = std::make_shared<iimm_t>();
          t_->imm = num_;
          (*now).back().args.push_back(t_);
        } else {
          double num_  = atof(tmp_);
          std::shared_ptr<fimm_t> t_ = std::make_shared<fimm_t>();
          t_->imm = num_;
          (*now).back().args.push_back(t_);
        }
      }
    } else if (str2int64(input_str) > 0) {
      int i = 0;
      for (i = 0; i < strlen(input_str); i++) {
        if (input_str[i] <= '9' and input_str[i] >= '0')
            continue;
        if (input_str[i] == 's')
            break;
      }
      std::shared_ptr<output_t> output_ = std::make_shared<output_t>();
      output_->content = std::string(input_str + i + 1);
      // std::cout << output_->content << std::endl;
      (*now).back().args.push_back(output_);
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
    for (size_t j = 0; j < inst[i].args.size(); j++) {
       std::cout << " ";
       inst[i].args[j]->visualizing();
    }
    std::cout << std::endl;
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

int main(int argc, char** argv) {
  controller_t controller;
  readraw(controller.mq, controller.exq, (argc > 1 ? argv[1] : "eop.out"));
  controller.exq.insert(controller.exq.begin(), controller.mq.begin(), controller.mq.end());
  for (auto&& p : coroutine_t::list())
    p->operator()();
  while(!eq.empty()) {
    auto e = eq.top(); eq.pop();
    if (global_time <= e.time) {
      global_time = e.time;
      e.callback->operator()();
    }
  }
  std::cout << "[" << global_time << "] " << "processor halted." << std::endl;
/*
  std::vector<inst_t> main_inst;
  std::vector<inst_t> except_inst;
  readraw(main_inst, except_inst, std::string("eop.out").c_str());
  printf("%d %d\n", main_inst.size(), except_inst.size());
  checkinput(main_inst, except_inst);
*/
  return 0;
}
