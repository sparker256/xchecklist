#ifndef PARSER__H
#define PARSER__H

#include <string>
#include <vector>
#include <iostream>
#include <map>

#include "interface.h"

typedef enum {XC_NOT, XC_EQ, XC_LT, XC_LE, XC_GT, XC_GE, XC_IN, XC_HYST,
              XC_AND, XC_OR, XC_POS_DIF, XC_NEG_DIF, XC_ABS_DIF} operation_t;
typedef enum {INACTIVE, SAY_LABEL, CHECKABLE, PROCESSING, SAY_SUFFIX, NEXT} item_state_t;
class checklist_binder;
extern checklist_binder *binder;
class checklist;
class dataref_t;
extern checklist *current_checklist;

#if LIN | APL
extern "C" bool voice_state;
#endif

extern char* chkltext;
extern int chkllineno;
extern int chkldebug;
extern FILE* chklin;

class checklist_item{
  friend std::ostream& operator<<(std::ostream &output, const checklist_item& s);
 public:
  checklist_item():index(-1), state(INACTIVE), checked(false){};
  virtual void print(std::ostream &output)const = 0;
  virtual ~checklist_item(){};
  virtual bool getDesc(checklist_item_desc_t &desc) = 0;
  virtual bool activate() = 0;
  virtual bool do_processing(bool copilotOn) = 0;
  virtual bool show(bool &val){(void) val; return false;};
  virtual void reset(){};
  void setIndex(int i){index = i;};
  bool item_done(){return (state == NEXT);};
  virtual bool check();
 protected:
  int index;
  item_state_t state;
  bool checked;
};


class checklist{
  friend std::ostream& operator<<(std::ostream &output, const checklist& dsc);
 public:
  checklist(std::string display, std::string menu);
  checklist(std::string display);
  ~checklist();
  bool add_item(checklist_item *i);
  void set_width(int f);
  bool activate(int index, bool force = false);
  bool item_checked(int item);
  bool do_processing(bool copilotOn);
  bool restart_checklist();
  bool activate_next_item(bool init = false);
  const std::string& get_name()const;
  const std::string& get_display_text()const;
  bool triggered();
  bool checklist_finished(bool *switchNext);
  void add_continue_flag(std::string label = "", dataref_t *dref = NULL);
  bool get_next(std::string &l){l = continue_label; return continue_flag;};
  void check_references(const std::map<std::string, int> &labels);
 private:
  std::string displaytext;
  std::string menutext;
  std::vector<checklist_item *> items;
  int width;
  int current_item;
  bool finished;
  bool trigger_block;
  std::vector<std::pair<std::string, dataref_t *> > gotos;
  bool continue_flag;
  std::string continue_label;
};

//Collection of checklists
class checklist_binder{
  friend std::ostream& operator<<(std::ostream &output, const checklist_binder& b);
  public:
    checklist_binder():current(-1){};
    ~checklist_binder();
    void add_checklist(checklist *c);
    bool select_checklist(unsigned int index, bool force = false);
    bool prev_checklist();
    bool next_checklist(bool followSwCont);
    bool item_checked(int item);
    bool do_processing(bool visible, bool copilotOn);
    bool get_checklist_names(int *all_checklists, int *menu_size, constname_t *names[], int *indexes[]);
    bool free_checklist_names(int all_checklists, int menu_size, constname_t *names[], int *indexes[]);
    bool checklist_finished(bool *switchNext);
    bool check_references();
  private:
    std::vector<checklist*> checklists;
    std::map<std::string, int> labels;
    int current;
};

class value{
  friend std::ostream& operator<<(std::ostream &output, const value& v);
  value_type_t value_type;
 public:
  value():value_type(TYPE_DOUBLE){};
  virtual ~value(){};
  virtual bool get_value(double &d)const = 0;
  virtual void print(std::ostream &output)const{output << *this;};
  value_type_t get_type()const{return value_type;};
  std::string get_type_str()const;
  void set_type(value_type_t t){value_type = t;};
  void cast(double &val)const;
  /*
  bool lt(const double &val1)const;
  bool gt(const double &val1)const;
  bool le(const double &val1)const;
  bool ge(const double &val1)const;
  bool eq(const double &val1)const;
  */
};

typedef bool (*func_ptr_t)(const std::vector<value *> *params, double &res);

class procedure:public value{
  friend std::ostream& operator<<(std::ostream &output, const procedure& v);
 public:
  procedure(std::string n, std::vector<value *> *par);
  virtual ~procedure();
  virtual bool get_value(double &d)const;
  virtual void print(std::ostream &output)const{output << *this;};
 private:
  std::string name;
  std::vector<value *> *params;
  func_ptr_t actor;
  static std::map<std::string, func_ptr_t> functions;
};

class dataref_name:public value{
  friend std::ostream& operator<<(std::ostream &output, const dataref_name& dn);
 public:
  dataref_name(std::string n, std::string i);
  dataref_name(std::string n);
  ~dataref_name();
  dataref_p getDataref();
  virtual bool get_value(double &d)const;
  virtual void print(std::ostream &output)const{output << *this;};
 private:
  std::string name;
  int index;
  dataref_p dataref_struct;
};

class arith_op : public value{
  friend std::ostream& operator<<(std::ostream &output, const arith_op& n);
 public:
  arith_op(value *val1, char op, value *val2);
  ~arith_op(){delete value1; delete value2;};
  virtual bool get_value(double &d)const;
  virtual void print(std::ostream &output)const{output << *this;};
 private:
  value *value1, *value2;
  char operation;
};

class number : public value{
  friend std::ostream& operator<<(std::ostream &output, const number& n);
 public:
  number(std::string i, std::string d, std::string e);
  virtual bool get_value(double &d)const;
  virtual void print(std::ostream &output)const{output << *this;};
 private:
  double get_precision(std::string &i, std::string &d, std::string &e);
  double value;
  double precision;
};

class dataref_t{
  friend std::ostream& operator<<(std::ostream &output, const dataref_t& d);
 public:
  virtual ~dataref_t(){};
  virtual bool registerDsc() = 0;
  virtual void reset_trig() = 0;
  virtual bool trigered() = 0;
  virtual void print(std::ostream &output)const;
};

class dataref_op : public dataref_t {
  friend std::ostream& operator<<(std::ostream &output, const dataref_op& d);
 public:
  dataref_op(dataref_t *dr1, operation_t o, dataref_t *dr2);
  virtual ~dataref_op();
  virtual bool registerDsc();
  virtual void reset_trig();
  virtual bool trigered();
  virtual void print(std::ostream &output)const{output << *this;};
 private:
  dataref_t *dref1;
  dataref_t *dref2;
  operation_t op;
};


class dataref_dsc : public dataref_t{
  friend std::ostream& operator<<(std::ostream &output, const dataref_dsc& d);
 public:
  dataref_dsc(dataref_name *dr, value *val);
  dataref_dsc(dataref_name *dr, operation_t *o, value *val);
  dataref_dsc(dataref_name *dr, value *v1, value *v2, bool plain_in = true);
  virtual ~dataref_dsc();
  virtual bool registerDsc();
  virtual void reset_trig(){state = NONE;};
  virtual bool trigered();
  virtual void print(std::ostream &output)const{output << *this;};
 private:
  bool checkTrig();
  dataref_name *data_ref;
  value *val1;
  value *val2;
  operation_t op;
  enum {NONE, INIT, TRIG} state;
  double ref_val;
};

class item_label{
  friend std::ostream& operator<<(std::ostream &output, const item_label& l);
 public:
  item_label(std::string label_left, std::string label_right);
  item_label(std::string label_left);
  bool getDesc(checklist_item_desc_t &desc);
  void say_label();
  void say_suffix();
 private:
  std::string label;
  std::string suffix;
};

class show_item: public checklist_item{
 public:
  show_item(dataref_t *d);
  virtual ~show_item(){delete dataref;};
  virtual void print(std::ostream &output)const;
  virtual bool getDesc(checklist_item_desc_t &desc);
  virtual bool activate(){return false;};
  virtual bool do_processing(bool copilotOn){(void) copilotOn; return false;};
  virtual bool show(bool &val);
  virtual void reset();
 private:
  dataref_t *dataref;
};

class void_item:public checklist_item{
  public:
    void_item(std::string s);
    void_item(std::string s, std::string s1);
    virtual ~void_item(){};
    virtual void print(std::ostream &output)const;
    virtual bool getDesc(checklist_item_desc_t &desc);
    virtual bool activate(){return false;};
    virtual bool do_processing(bool copilotOn){(void) copilotOn; return false;};
  private:
    std::string text;
    std::string text1;
};

class remark_item:public checklist_item{
  public:
    remark_item(std::string s);
    virtual ~remark_item(){};
    virtual void print(std::ostream &output)const;
    virtual bool getDesc(checklist_item_desc_t &desc);
    virtual bool activate();
    virtual bool do_processing(bool copilotOn);
  private:
    std::string text;
};

class chk_item:public checklist_item{
  public:
    chk_item(item_label *l, dataref_t *d, bool ch);
    virtual ~chk_item();
    virtual void print(std::ostream &output)const;
    virtual bool getDesc(checklist_item_desc_t &desc);
    virtual bool activate();
    virtual bool do_processing(bool copilotOn);
    virtual bool check();
  private:
    item_label *label;
    dataref_t *dataref;
    bool checkable;
    //bool check_state;
};



bool parse_clist(const std::string &fname, int debug);

#endif
