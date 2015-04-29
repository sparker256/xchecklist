#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <iostream>
#include <ostream>
#include "chkl_parser.h"
#include "speech.h"
#include "utils.h"
#include <XPLMUtilities.h>


#if IBM
float roundf(float x)
{
  return floorf(x + 0.5f);
}
#endif

int chklparse(void);

//#if IBM
//  int chkllineno;
//#endif

checklist *current_checklist;
char *parsed_file;

bool checklist_item::check()
{
  if(state == PROCESSING){
    checked = true;
    return true;
  }
  return false;
}

checklist::checklist(std::string display, std::string menu)
{
  displaytext = display;
  menutext = menu;
  width = 300;
  finished = false;
  trigger_block = false;
  continueFlag = false;
}

checklist::checklist(std::string display)
{
  displaytext = display;
  menutext = "";
  width = 300;
  finished = false;
  trigger_block = false;
  continueFlag = false;
}

checklist::~checklist()
{
  std::vector<checklist_item*>::iterator i;
  for(i = items.begin(); i != items.end(); ++i){
    delete(*i);
  }
}

bool checklist::add_item(checklist_item *i)
{
  items.push_back(i);
  return true;
}

void checklist::set_width(int w)
{
  width = w;
}

template<class T>
    T fromString(const std::string& s)
    {
      std::istringstream stream (s);
      T t;
      stream >> t;
      return t;
    }


float number::get_precision(std::string &i, std::string &d, std::string &e)
{
  (void) i;
  int sig = 1;
  if(d.size() > 1){
    sig = (d.size() - 1);
  }
  float bnd = pow(0.1, sig);
  if(!e.empty()){
    std::ostringstream str;
    str<<bnd<<e.c_str();
    bnd = fromString<float>(str.str());
  }
  //std::cout<<i<<d<<e<<" => precision "<<bnd<<std::endl;
  return bnd;
}


number::number(std::string i, std::string d, std::string e)
{
  precision = get_precision(i, d, e);
  value = fromString<float>(i + d + e);
/*
  if(d.size() > 1){
    decimals = d.size() - 1; //there is a dot before the decimals
  }else{
    decimals = 0;
  }
  frexpf(value, &exp); // get exponent... 
*/
};

float number::get_value()
{
  return value;
}

std::ostream& operator<<(std::ostream &output, const number& n)
{
  return output<<n.value;
}

std::ostream& operator<<(std::ostream &output, const checklist& dsc)
{
  output<<"CHECKLIST: "<<dsc.displaytext.c_str()<<", Menu: "<<dsc.menutext.c_str()<<std::endl;
  std::vector<checklist_item*>::const_iterator i;
  for(i = dsc.items.begin(); i != dsc.items.end(); ++i){
    output<<**i;
  }
  return output;
}

std::ostream& operator<<(std::ostream &output, const checklist_binder& b)
{
  output<<"BINDER: "<<std::endl;
  std::vector<checklist*>::const_iterator i;
  for(i = b.checklists.begin(); i != b.checklists.end(); ++i){
    output<<**i;
  }
  return output;
}

std::ostream& operator<<(std::ostream &output, const dataref_name& dn)
{
  output<<"Dataref: "<<dn.name.c_str();
  if(dn.index != -1){
    output<<"["<<dn.index<<"]";
  }
  return output;
}

std::ostream& operator<<(std::ostream &output, const dataref_t& d)
{
  (void) d;
  output<<" *SOMETHING IS NOT OK HERE, CONTACT DEVELOPER PLEASE* ";
  return output;
}

std::ostream& operator<<(std::ostream &output, const dataref_op& d)
{
  output<<" ("<<*(d.dref1);
  if(d.op == XC_AND) output<<" & ";
  if(d.op == XC_OR)  output<<" | ";
  output<<*(d.dref2)<<") ";
  return output;
}

std::ostream& operator<<(std::ostream &output, const dataref_dsc& d)
{
  output<<*(d.data_ref);
  switch(d.op){
    case XC_NOT:
      output<<" != "<<*(d.val1);
      break;
    case XC_EQ:
      output<<" == "<<*(d.val1);
      break;
    case XC_LT:
      output<<" < "<<*(d.val1);
      break;
    case XC_GT:
      output<<" > "<<*(d.val1);
      break;
    case XC_LE:
      output<<" <= "<<*(d.val1);
      break;
    case XC_GE:
      output<<" >= "<<*(d.val1);
      break;
    case XC_IN:
      output<<" BELONGS TO ("<< *(d.val1) <<" : "<<*(d.val2)<<")";
      break;
    case XC_HYST:
      output<<" HYST ("<< *(d.val1) <<" : "<<*(d.val2)<<")";
      break;
    default:
      output<<" *SOMETHING IS NOT OK HERE, CONTACT DEVELOPER PLEASE* ";
      break;
  }
  return output;
}
  
std::ostream& operator<<(std::ostream &output, const item_label& l)
{
  output<<l.label.c_str();
  if(!l.suffix.empty()){
    output<<"    "<<l.suffix.c_str();
  }
  return output;
}

std::ostream& operator<<(std::ostream &output, const checklist_item& s)
{
  s.print(output);
  return output;
}

void show_item::print(std::ostream &output)const
{
  output<<"SW_SHOW: "<<*dataref<<std::endl;
}

void void_item::print(std::ostream &output)const
{
  output<<"SW_VOID: "<<text.c_str()<<std::endl;
}

void remark_item::print(std::ostream &output)const
{
  output<<"SW_REMARK: "<<text.c_str()<<std::endl;
}

void chk_item::print(std::ostream &output)const
{
  if(checkable){
    output<<"SW_ITEM: ";
  }else{
    output<<"SW_INFO_ITEM: ";
  }
  output<<*label;
  if(dataref != NULL){
    output<<" "<<*dataref;
  }
  output<<std::endl;
}

dataref_name::dataref_name(std::string n, std::string i)
{
  name = n;
  index = atoi(i.c_str());
  dataref_struct = NULL;
}

dataref_name::dataref_name(std::string n)
{
  name = n;
  index = -1;
  dataref_struct = NULL;
}

dataref_dsc::dataref_dsc(dataref_name *dr, number *val)
{
  data_ref = dr;
  val1 = val;
  val2 = NULL;
  op = XC_EQ;
}

dataref_dsc::dataref_dsc(dataref_name *dr, operation_t *o, number *val)
{
  state = NONE;
  data_ref = dr;
  val1 = val;
  val2 = NULL;
  op = *o;
}

dataref_dsc::dataref_dsc(dataref_name *dr, number *v1, number *v2, bool plain_in)
{
  state = NONE;
  data_ref = dr;
  val1 = v1;
  val2 = v2;
  if(plain_in){
    op = XC_IN;
  }else{
    op = XC_HYST;
  }
}

dataref_dsc::~dataref_dsc()
{
  if(val1 != NULL){
    delete(val1);
    val1 = NULL;
  }
  if(val2 != NULL){
    delete(val2);
    val2 = NULL;
  }
  delete data_ref;
}

dataref_name::~dataref_name()
{
  if(dataref_struct != NULL){
    dispose_dataref(&dataref_struct);
  }
}

dataref_p dataref_name::getDataref()
{
  if(dataref_struct != NULL){
    return dataref_struct;
  }
  if(index < 0){
    if(find_dataref(name.c_str(), &dataref_struct)){
      return dataref_struct;
    }else{
      return NULL;
    }
  }else{
    if(find_array_dataref(name.c_str(), index, &dataref_struct)){
      return dataref_struct;
    }else{
      return NULL;
    }
  }
  return NULL;
}

bool dataref_dsc::registerDsc()
{
  dataref_struct = data_ref->getDataref();
//  get_dataref_type(dataref_struct);
  return true;
}

bool number::le(float &val1)
{
  return val1 <= value;
}

bool number::ge(float &val1)
{
  return val1 >= value;
}

bool number::lt(float &val1)
{
  return val1 < value;
}

bool number::gt(float &val1)
{
  return val1 > value;
}

bool number::eq(float &val1)
{
  float tmp = precision * roundf(val1 / precision);
  return fabsf(value - tmp) < (precision / 10.0f);
}

bool operator<=(float &val1, number val2)
{
  return val2.le(val1);
}

bool operator>=(float &val1, number val2)
{
  return val2.ge(val1);
}

bool operator<(float &val1, number val2)
{
  return val2.lt(val1);
}

bool operator>(float &val1, number val2)
{
  return val2.gt(val1);
}

bool operator==(float &val1, number val2)
{
  return val2.eq(val1);
}


bool dataref_dsc::checkTrig(float val)
{
  if(val1->get_value() < val2->get_value()){
    switch(state){
      case NONE:
      case TRIG:
        if(val <= *val1){
          state = INIT;
        }
        break;
      case INIT:
        if(val >= *val2){
          state = TRIG;
        }
        break;
    }
  }else{
    switch(state){
      case NONE:
      case TRIG:
      	if(val >= *val1){
          state = INIT;
        }
        break;
      case INIT:
        if(val <= *val2){
          state = TRIG;
        }
        break;
    }
  }
  if(state == TRIG){
    return true;
  }
  return false;
}


bool dataref_dsc::trigered()
{
  bool res = false; 
  if(dataref_struct == NULL){
    return res;
  }
  float val = get_float_dataref(dataref_struct);
  switch(op){
    case XC_NOT:
      res = (val == *val1) ? false : true;
      break;
    case XC_EQ:
      res = (val == *val1) ? true : false;
      break;
    case XC_LT:
      res = (val < *val1) ? true : false;
      break;
    case XC_GT:
      res = (val > *val1)? true : false;
      break;
    case XC_LE:
      res = (val <= *val1) ? true : false;
      break;
    case XC_GE:
      res = (val >= *val1)? true : false;
      break;
    case XC_IN:
      res = ((val >= *val1) && (val <= *val2 )) ? true : false;
      break;
    case XC_HYST:
      res = checkTrig(val);
      break;
    default:
      res = false;
      break;
  }
  return res;
}



item_label::item_label(std::string label_left, std::string label_right)
{
  label = label_left;
  suffix = label_right;
}

item_label::item_label(std::string label_left)
{
  label = label_left;
  suffix = "CHECK";
}

void item_label::say_label()
{
    if(voice_state) {
        say(label.c_str());
    }
}

void item_label::say_suffix()
{
   if(voice_state) {
      say(suffix.c_str());
   }
}

void_item::void_item(std::string s)
{
  text = s;
}

remark_item::remark_item(std::string s)
{
  text = s;
}

chk_item::chk_item(item_label *l, dataref_t *d, bool ch)
{
  label = l;
  dataref = d;
  checkable = ch;
}

chk_item::~chk_item()
{
  delete label;
  delete dataref;
}

checklist_binder::~checklist_binder()
{
  std::vector<checklist*>::iterator i;
  for(i = checklists.begin(); i != checklists.end(); ++i){
    delete(*i);
    *i = NULL;
  }
}

void checklist_binder::add_checklist(checklist *c)
{
  checklists.push_back(c);
}

bool checklist_binder::select_checklist(unsigned int index, bool force)
{
  //Unsigned must be >= 0...
  if(index >= checklists.size()) return false;
  current = index;
  
  return checklists[current]->activate(current, force);
}

bool checklist_binder::prev_checklist()
{
  return select_checklist(current - 1);
}

bool checklist_binder::next_checklist()
{
  return select_checklist(current + 1);
}

bool checklist_binder::item_checked(int item)
{
  return checklists[current]->item_checked(item);
}

show_item::show_item(dataref_t *d):dataref(d)
{
  if(dataref != NULL){
    dataref->registerDsc();
  }
}

bool show_item::show(bool &val)
{
  if(dataref != NULL){
    val = dataref->trigered();
    return true;
  }
  val = false;
  return false;
}

void show_item::reset()
{
  if(dataref != NULL){
    dataref->reset_trig();
  }
}

bool checklist::triggered()
{
  bool res = true;
  bool found_trig = false;
  bool trigger = true;
  
  for(unsigned int i = 0; i < items.size(); ++i){
    found_trig |= items[i]->show(res);
    trigger &= res;
  }
  
  if(found_trig){ //there is at least one trigger
    if(trigger_block){
      trigger_block = trigger;
      return false;
    }
    return trigger;
  }else{
    return false;
  }
}

bool checklist_binder::do_processing(bool visible, bool copilotOn)
{
  int triggered = -1;
  for(unsigned int i = 0; i < checklists.size(); ++i){
    if(checklists[i]->triggered()){
      //We'll take the first one that triggers
      if(triggered == -1){
        triggered = i;
        printf("Checklist No. %d triggers!\n", i);
      }
    }
  }
  
  if(visible){
    return checklists[current]->do_processing(copilotOn);
  }else{
    if(triggered >= 0){
      select_checklist(triggered, true);
    }
  }
  return true;
}

bool checklist::do_processing(bool copilotOn)
{
  if(current_item > -1){
    items[current_item]->do_processing(copilotOn);
    if(items[current_item]->item_done()){
      activate_next_item();
    }
  }
  return true;
}

bool chk_item::activate()
{
  state = INACTIVE;
  return true;
}

bool remark_item::activate()
{
  state = INACTIVE;
  return true;
}

bool chk_item::do_processing(bool copilotOn)
{
    static float elapsed = 0.0f;
    //printf("State: %d\n", state);
    switch(state){
    case INACTIVE:
        elapsed = 0.0f;
        if(speech_active()){
            label->say_label();
            state = SAY_LABEL;
        }else{
            state = CHECKABLE;
        }
        break;
    case SAY_LABEL:
        elapsed += 0.1f; // interval the flight loop is set to
        if(spoken(elapsed)){
            state = CHECKABLE;
        }
        break;
    case CHECKABLE:
        elapsed = 0;
        activate_item(index);
        checked = false;
        state = PROCESSING;
        break;
    case PROCESSING:
        elapsed = 0;
        if(checked || 
           (copilotOn && (dataref != NULL) && dataref->trigered())){
          if(speech_active()){
            label->say_suffix();
            state = SAY_SUFFIX;
          }else{
            state = NEXT;
          }
          check_item(index);
        }
        break;
    case SAY_SUFFIX:
        elapsed += 0.1f; // interval the flight loop is set to
        if(spoken(elapsed)){
            state = NEXT;
        }
        break;
    case NEXT:
        elapsed = 0;
        break;
    default:
        elapsed = 0;
        state = INACTIVE; //defensive
        break;
    }
  return true;
}

bool remark_item::do_processing(bool copilotOn)
{
    (void) copilotOn;
    static float elapsed = 0.0f;
    //printf("State: %d\n", state);
    switch(state){
    case INACTIVE:
        elapsed = 0.0f;
        if(speech_active()){
            if(voice_state) {
              say(text.c_str());
            }
            state = SAY_LABEL;
        }else{
            state = NEXT;
        }
        break;
    case SAY_LABEL:
        elapsed += 0.1f; // interval the flight loop is set to
        if(spoken(elapsed)){
            state = NEXT;
        }
        break;
    case NEXT:
        elapsed = 0;
        break;
    default:
        elapsed = 0;
        state = INACTIVE; //defensive
        break;
    }
  return true;
}

bool chk_item::check()
{
  if(checkable && (state == PROCESSING)){
    checked = true;
    return true;
  }
  return false;
}

//Might be usefull for speech stuff...
bool checklist::item_checked(int item)
{
  (void) item;
  //Current item is checked, find the next checkable/activatable item
  if(current_item > -1){
    return items[current_item]->check();
  }
  return false;
}

bool checklist::activate_next_item(bool init)
{
  if(init){
    current_item = -1;
    finished = false;
  }
  ++current_item;
  while(1){
    if((unsigned int)current_item < items.size()){
      if(items[current_item]->activate()){
	return true;
      }
    }else{
      finished = true;
      current_item = -1;
      return false;
    }
    ++current_item;
  }
  return false;
}

bool checklist::activate(int index, bool force)
{
  if(triggered()){
    trigger_block = true; 
    for(unsigned int i = 0; i < items.size(); ++i){
      items[i]->reset();
    }
  }
  
  checklist_item_desc_t* desc = NULL;
  desc = new checklist_item_desc_t[items.size()];
  int j = 0;
  for(unsigned int i = 0; i < items.size(); ++i){
    if(items[i]->getDesc(desc[j])){
      items[i]->setIndex(j);
      ++j;
    }else{
      items[i]->setIndex(-1);
    }
  }
  
  bool res = create_checklist(j, displaytext.c_str(), desc, width, index, force);
  delete [] desc;
  activate_next_item(true);
  return res;
}

const std::string& checklist::get_name()const
{
  return menutext;
}

bool checklist_binder::get_checklist_names(int *all_checklists, int *menu_size, constname_t *names[], int *indexes[])
{
  int sizeOfAll = checklists.size();
  *names = new constname_t[sizeOfAll];
  *indexes = new int[sizeOfAll];
  int j = 0;
  for(int i = 0; i < sizeOfAll; ++i){
    if(!checklists[i]->get_name().empty()){
      (*names)[j] = (constname_t)checklists[i]->get_name().c_str();
      (*indexes)[j] = i;
      ++j;
    }
  }
  *menu_size = j;
  *all_checklists = sizeOfAll;
  return true;
}

bool checklist_binder::free_checklist_names(int all_checklists, int menu_size, constname_t *names[], int *indexes[])
{
  (void) all_checklists;
  (void) menu_size;
  delete [] *names;
  delete [] *indexes;
  *names = NULL;
  *indexes = NULL;
  return true;
}



bool void_item::getDesc(checklist_item_desc_t &desc)
{
  desc.text = text.c_str();
  desc.suffix = (char *)"";
  desc.info_only = true;
  desc.item_void = true;
  desc.copilot_controlled = false;
  return true;
}

bool remark_item::getDesc(checklist_item_desc_t &desc)
{
  desc.text = text.c_str();
  desc.suffix = (char *)"";
  desc.info_only = true;
  desc.item_void = true;
  desc.copilot_controlled = false;
  return true;
}

bool item_label::getDesc(checklist_item_desc_t &desc)
{
  desc.text = label.c_str();
  if(!suffix.empty()){
    desc.suffix = suffix.c_str();
  }else{
    desc.suffix = "CHECK";
  }
  return true;
}

bool chk_item::getDesc(checklist_item_desc_t &desc)
{
  desc.info_only = checkable ? false : true;
  desc.item_void = false;
  if(dataref != NULL){
    dataref->registerDsc();
    desc.copilot_controlled = true;
  }else{
    desc.copilot_controlled = false;
  }
  return label->getDesc(desc);
}

bool show_item::getDesc(checklist_item_desc_t &desc)
{
  (void) desc;
  return false;
}


bool parse_clist(const std::string &fname, int debug)
{
  if((chklin=fopen(fname.c_str(), "r")) != NULL){
    parsed_file = strdup(fname.c_str());
    chkldebug=debug;
    
    current_checklist = NULL;
    binder = NULL;
    int res = chklparse();
    fclose(chklin);
    free(parsed_file);
    parsed_file = NULL;
    chkllineno = 1;
    if(res == 0){
        xcDebug("Xchecklist: Preferences read OK!\n");
        return(true);
    }
  }
  xcDebug("Xchecklist: Error encountered while reading preferences!\n");
  return(false);
}

void chklerror(char const *s)
{
  xcDebug("Xchecklist: %s in file %s, line %d near '%s'\n",
          s, parsed_file, chkllineno, chkltext);
}


bool checklist_binder::checklist_finished(bool *switchNext)
{
    if((current < 0) || (current > ((int)checklists.size() - 1))){
        return false;
    }else{
        return checklists[current]->checklist_finished(switchNext);
    }
    return false;
}

bool checklist::checklist_finished(bool *switchNext)
{
    *switchNext = continueFlag;
    return finished;
}

dataref_op::dataref_op(dataref_t *dr1, operation_t o, dataref_t *dr2) : dref1(dr1), dref2(dr2), op(o)
{
  dref1->registerDsc();
  dref2->registerDsc();
}

dataref_op::~dataref_op()
{
  if(dref1 != NULL){
    delete dref1;
  }
  if(dref2 != NULL){
    delete dref2;
  }
}

bool dataref_op::registerDsc()
{
  return true;
}

void dataref_op::reset_trig()
{
  dref1->reset_trig();
  dref2->reset_trig();
}

bool dataref_op::trigered()
{
  if(op == XC_AND){
    return(dref1->trigered() && dref2->trigered());
  }
  if(op == XC_OR){
    return(dref1->trigered() || dref2->trigered());
  }
  return false;
}
