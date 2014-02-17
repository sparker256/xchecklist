#ifndef PARSER__H
#define PARSER__H

#include <string>
#include <vector>
#include <iostream>

#include "interface.h"

typedef enum {XC_NOT, XC_EQ, XC_LT, XC_LE, XC_GT, XC_GE, XC_IN, XC_HYST} operation_t;
typedef enum {INACTIVE, SAY_LABEL, CHECKABLE, PROCESSING, SAY_SUFFIX, NEXT} item_state_t;
class checklist_binder;
extern checklist_binder *binder;

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
  void check(){if(state == PROCESSING) checked = true;};
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
  bool triggered();
  bool checklist_finished();
 private:
  std::string displaytext;
  std::string menutext;
  std::vector<checklist_item *> items;
  int width;
  int current_item;
  bool finished;
  bool trigger_block;
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
    bool next_checklist();
    bool item_checked(int item);
    bool do_processing(bool visible, bool copilotOn);
    bool get_checklist_names(int *all_checklists, int *menu_size, constname_t *names[], int *indexes[]);
    bool free_checklist_names(int all_checklists, int menu_size, constname_t *names[], int *indexes[]);
    bool checklist_finished();
  private:
    std::vector<checklist*> checklists;
    int current;
};

class dataref_name{
  friend std::ostream& operator<<(std::ostream &output, const dataref_name& dn);
 public:
  dataref_name(std::string n, std::string i);
  dataref_name(std::string n);
  ~dataref_name();
  dataref_p getDataref();
 private:
  std::string name;
  int index;
  dataref_p dataref_struct;
};

class number{
  friend std::ostream& operator<<(std::ostream &output, const number& n);
 public:
  number(std::string i, std::string d, std::string e);
  float get_value();
  bool lt(float &val1);
  bool gt(float &val1);
  bool le(float &val1);
  bool ge(float &val1);
  bool eq(float &val1);
 private:
  float get_precision(std::string &i, std::string &d, std::string &e);
  float value;
  float precision;
};

class dataref_dsc{
  friend std::ostream& operator<<(std::ostream &output, const dataref_dsc& d);
 public:
  dataref_dsc(dataref_name *dr, number *val);
  dataref_dsc(dataref_name *dr, operation_t *o, number *val);
  dataref_dsc(dataref_name *dr, number *v1, number *v2, bool plain_in = true);
  ~dataref_dsc();
  bool registerDsc();
  void reset_trig(){state = NONE;};
  bool trigered();
 private:
  bool checkTrig(float val);
  dataref_name *data_ref;
  number *val1;
  number *val2;
  operation_t op;
  enum {NONE, INIT, TRIG} state;
  dataref_p dataref_struct;
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
  show_item(dataref_dsc *d);
  virtual ~show_item(){delete dataref;};
  virtual void print(std::ostream &output)const;
  virtual bool getDesc(checklist_item_desc_t &desc);
  virtual bool activate(){return false;};
  virtual bool do_processing(bool copilotOn){(void) copilotOn; return false;};
  virtual bool show(bool &val);
  virtual void reset();
 private:
  dataref_dsc *dataref;
};

class void_item:public checklist_item{
  public:
    void_item(std::string s);
    virtual ~void_item(){};
    virtual void print(std::ostream &output)const;
    virtual bool getDesc(checklist_item_desc_t &desc);
    virtual bool activate(){return false;};
    virtual bool do_processing(bool copilotOn){(void) copilotOn; return false;};
  private:
    std::string text;
};

class chk_item:public checklist_item{
  public:
    chk_item(item_label *l, dataref_dsc *d, bool ch);
    virtual ~chk_item();
    virtual void print(std::ostream &output)const;
    virtual bool getDesc(checklist_item_desc_t &desc);
    virtual bool activate();
    virtual bool do_processing(bool copilotOn);
  private:
    item_label *label;
    dataref_dsc *dataref;
    bool checkable;
    bool check_state;
};



bool parse_clist(const std::string &fname, int debug);

#endif
