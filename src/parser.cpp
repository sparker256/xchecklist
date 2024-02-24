#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <iostream>
#include <ostream>
#include <map>
#include "chkl_parser.h"
#include "speech.h"
#include "utils.h"
#include <XPLMUtilities.h>


#if IBM
double round(double x)
{
  return floor(x + 0.5f);
}
#endif

int chklparse(void);

//#if IBM
//  int chkllineno;
//#endif

checklist *current_checklist;
char *parsed_file;
static bool debug_expr_eval;

palette p;

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
  menutext = menu;
  displaytext = display;
  width = 300;
  true_width = 0;
  true_width_label = 0;
  true_width_suffix = 0;
  finished = false;
  trigger_block = false;
  continue_flag = false;
  continue_label = "";
  muted = false;
}

void checklist::mute()
{
  muted = true;
}

checklist::~checklist()
{
  std::vector<checklist_item*>::iterator i;
  for(i = items.begin(); i != items.end(); ++i){
    delete(*i);
  }
  for(size_t j = 0; j < gotos.size(); ++j){
    delete(gotos[j].second);
  }
}

bool checklist::add_item(checklist_item *i)
{
  items.push_back(i);
  checklist_item_desc_t desc;
  if(i->getDesc(desc)){
    int tmp_w_l = measure_string(desc.text, strlen(desc.text));
    int tmp_w_s = measure_string(desc.suffix, strlen(desc.suffix));
    if(tmp_w_l > true_width_label){
      true_width_label = tmp_w_l;
    }
    if(tmp_w_s > true_width_suffix){
      true_width_suffix = tmp_w_s;
    }
    true_width = true_width_label + true_width_suffix;
  }
  if(muted){
    if(chk_item *item = dynamic_cast<chk_item*>(i)){
      item->reverse_silent();
    }
  }
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


double number::get_precision(std::string &i, std::string &d, std::string &e)
{
  (void) i;
  int sig = 1;
  if(d.size() > 1){
    sig = (d.size() - 1);
  }
  double bnd = pow(0.1, sig);
  if(!e.empty()){
    std::ostringstream str;
    str<<bnd<<e.c_str();
    bnd = fromString<double>(str.str());
  }
  //std::cout<<i<<d<<e<<" => precision "<<bnd<<std::endl;
  return bnd;
}


number::number(std::string i, std::string d, std::string e)
{
  precision = get_precision(i, d, e);
  value = fromString<double>(i + d + e);
/*
  if(d.size() > 1){
    decimals = d.size() - 1; //there is a dot before the decimals
  }else{
    decimals = 0;
  }
  frexpf(value, &exp); // get exponent...
*/
}

bool number::get_value(double &d)const
{
  d = (double)value;
  cast(d);
  return true;
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
  d.print(output);
  return output;
}

void dataref_t::print(std::ostream &output)const
{
  output <<" * Trying to print a dataref_t, CONTACT DEVELOPER PLEASE * ";
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
    case XC_POS_DIF:
      output<<" VALUE GROWS MORE THAN "<<*(d.val1);
      break;
    case XC_NEG_DIF:
      output<<" VALUE DECREASES MORE THAN "<<*(d.val1);
      break;
    case XC_ABS_DIF:
      output<<" DIFFERENCE BIGGER THAN "<<*(d.val1);
      break;
    default:
      output<<" *SOMETHING IS NOT OK HERE, CONTACT DEVELOPER PLEASE (unknown operator)* ";
      break;
  }
  return output;
}

std::ostream& operator<<(std::ostream &output, const item_label& l)
{
  output<<l.label->c_str();
  if(!l.suffix->empty()){
    output<<"    "<<l.suffix->c_str();
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
  output<<"SW_VOID: "<<text->c_str()<<" "<<text1->c_str()<<std::endl;
}

void remark_item::print(std::ostream &output)const
{
  output<<"SW_REMARK: "<<text->c_str()<<std::endl;
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
  getDataref();
}

dataref_name::dataref_name(std::string n)
{
  index = -1;
  size_t pos = n.find_first_of('[');
  if(pos == n.npos){
    name = n;
  }else{
    name = n.substr(0, pos);
    std::istringstream idx_str(n.substr(pos + 1));
    idx_str >> index;
  }

  dataref_struct = NULL;
  getDataref();
}

bool dataref_name::get_value(double &d)const
{
  if(dataref_struct && (dataref_struct->accessor)){
    d = dataref_struct->accessor(dataref_struct);
    cast(d);
  }else{
    d = 0.0;
  }
  return true;
}

dataref_dsc::dataref_dsc(value *dr, value *val)
{
  data_ref = dr;
  val1 = val;
  val2 = NULL;
  op = XC_EQ;
  data_ref->set_type(val1->get_type());

  if(debug_expr_eval){
    double res;
    val1->get_value(res);
    std::cout << "DEBUG EXPRESSIONS: " << val1->get_type_str() << *val1 << " = " << res << std::endl;
  }
}

dataref_dsc::dataref_dsc(value *dr, operation_t *o, value *val)
{
  state = NONE;
  data_ref = dr;
  val1 = val;
  val2 = NULL;
  op = *o;
  data_ref->set_type(val1->get_type());

  if(debug_expr_eval){
    double res;
    val1->get_value(res);
    std::cout << "DEBUG EXPRESSIONS: " << val1->get_type_str() << *val1 << " = " << res << std::endl;
  }
}

dataref_dsc::dataref_dsc(value *dr, value *v1, value *v2, bool plain_in)
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
  data_ref->set_type(val1->get_type());

  if(debug_expr_eval){
    double res1, res2;
    val1->get_value(res1);
    val2->get_value(res2);
    std::cout << "DEBUG EXPRESSIONS: " << val1->get_type_str() << *val1 << " = " << res1 << std::endl;
    std::cout << "DEBUG EXPRESSIONS: " << val2->get_type_str() << *val2 << " = " << res2 << std::endl;
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
    if(find_dataref(name.c_str(), &dataref_struct, get_type())){
      return dataref_struct;
    }else{
      xcWarn("Dataref %s not found!\n", name.c_str());
      return NULL;
    }
  }else{
    if(find_array_dataref(name.c_str(), index, &dataref_struct, get_type())){
      return dataref_struct;
    }else{
      //xcDebug("Array dataref %s[%d] not found, trying Carenado style.\n", name.c_str(), index);
      //printf("Array dataref %s[%d] not found, trying Carenado style.\n", name.c_str(), index);
      //Lets try the Carenado style
      std::ostringstream strstr;
      strstr << name << "[" << index << "]";
      std::string str = strstr.str();
      if(find_dataref(str.c_str(), &dataref_struct, get_type())){
        return dataref_struct;
      }else{
        xcWarn("Dataref %s not found!\n", str.c_str());
        return NULL;
      }
      return NULL;
    }
  }
  return NULL;
}

bool dataref_dsc::registerDsc()
{
//  dataref_struct = data_ref->getDataref();
//  get_dataref_type(dataref_struct);
  return true;
}

bool operator<=(const value &val1, const value &val2)
{
  double v1, v2;
  val1.get_value(v1);
  val2.get_value(v2);
  return v1 <= v2;
}

bool operator>=(const value &val1, const value &val2)
{
  double v1, v2;
  val1.get_value(v1);
  val2.get_value(v2);
  return v1 >= v2;
}

bool operator<(const value &val1, const value &val2)
{
  double v1, v2;
  val1.get_value(v1);
  val2.get_value(v2);
  return v1 < v2;
}

bool operator>(const value &val1, const value &val2)
{
  double v1, v2;
  val1.get_value(v1);
  val2.get_value(v2);
  return v1 > v2;
}

bool operator==(const value &val1, const value &val2)
{
  double v1, v2;
  val1.get_value(v1);
  val2.get_value(v2);
  return fabs(v1 - v2) < 1e-6;
}

bool dataref_dsc::checkTrig()
{
  if(*val1 < *val2){
    switch(state){
      case NONE:
      case TRIG:
        if(*data_ref <= *val1){
          state = INIT;
        }
        break;
      case INIT:
        if(*data_ref >= *val2){
          state = TRIG;
        }
        break;
    }
  }else{
    switch(state){
      case NONE:
      case TRIG:
      	if(*data_ref >= *val1){
          state = INIT;
        }
        break;
      case INIT:
        if(*data_ref <= *val2){
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
  double val, value1, value2;
  //data_ref->get_value(val);
  //val1->get_value(value1);
  //std::cout<<"Cond: '"<<*this<<"'  Current value: "<<val<<std::endl;
  switch(op){
    case XC_NOT:
      res = !(*data_ref == *val1);
      break;
    case XC_EQ:
      res = *data_ref == *val1;
      break;
    case XC_LT:
      res = *data_ref < *val1;
      break;
    case XC_GT:
      res = *data_ref > *val1;
      break;
    case XC_LE:
      res = *data_ref <= *val1;
      break;
    case XC_GE:
      res = *data_ref >= *val1;
      break;
    case XC_IN:
      val2->get_value(value2);
      res = (*data_ref >= *val1) && (*data_ref <= *val2 );
      break;
    case XC_HYST:
      res = checkTrig();
      break;
    case XC_POS_DIF:
      data_ref->get_value(val);
      val1->get_value(value1);
      if(state != INIT){
        ref_val = val;
        state = INIT;
        //std::cout<<"POS_DIF ref = "<<ref_val<<std::endl;
      }
      res = ((val - ref_val) > value1);
      break;
    case XC_NEG_DIF:
      data_ref->get_value(val);
      val1->get_value(value1);
      if(state != INIT){
        ref_val = val;
        state = INIT;
        //std::cout<<"NEG_DIF ref = "<<ref_val<<std::endl;
      }
      res = ((ref_val - val) > value1);
      break;
    case XC_ABS_DIF:
      data_ref->get_value(val);
      val1->get_value(value1);
      if(state != INIT){
        ref_val = val;
        state = INIT;
        //std::cout<<"ABS_DIF ref = "<<ref_val<<std::endl;
      }
      res = (fabs(val - ref_val) > value1);
      break;
    default:
      res = false;
      break;
  }
  return res;
}



item_label::item_label(coloured_string *label_left, coloured_string *label_right)
{
  label = label_left;
  suffix = label_right;
}

item_label::item_label(coloured_string *label_left)
{
  label = label_left;
  suffix = new coloured_string("CHECK");
}

void item_label::say_label()
{
    if(voice_state) {
        say(label->c_str());
    }
}

void item_label::say_suffix()
{
   if(voice_state) {
      say(suffix->c_str());
   }
}

void_item::void_item(coloured_string *s)
{
  text = s;
  text1 = new coloured_string("");
}

void_item::void_item(coloured_string *s, coloured_string *s1)
{
  text = s;
  text1 = s1;
}

remark_item::remark_item(coloured_string *s)
{
  text = s;
}

chk_item::chk_item(item_label *l, dataref_t *d, bool ch, bool silent)
{
  label = l;
  dataref = d;
  checkable = ch;
  dont_speak = silent;
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
  std::string name = c->get_display_text();
  if(!name.empty()){
    if(labels.find(name) != labels.end()){
      xcWarn("Checklist label '%s' already used!\n", name.c_str());
      return;
    }
    labels[name] = checklists.size() - 1;
  }
}

bool checklist_binder::select_checklist(unsigned int index, bool force)
{
  //Unsigned must be >= 0...
  if(index >= checklists.size()){
    return false;
  }
  current = index;

  return checklists[current]->activate(current, force);
}

bool checklist_binder::prev_checklist()
{
  return select_checklist(current - 1, true);
}

bool checklist_binder::next_checklist(bool followSwCont)
{
  std::string label;
  if((current < 0) && ((size_t)current >= checklists.size())){
      return false;
  }
  if(checklists[current]->get_next(label) && followSwCont){
    if(labels.find(label) != labels.end()){
      return select_checklist(labels[label], true);
    }
    if(!label.empty()){
      xcWarn("Sw_continue requests to continue on nonexistent label '%s'.\n", label.c_str());
    }
  }
  return select_checklist(current + 1, true);
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
      //printf("Triggered!!!\n");
      //We'll take the first one that triggers
      if(triggered == -1){
        triggered = i;
        //printf("Checklist No. %d triggers!\n", i);
      }
    }
  }

  if(visible){
    if((current >= 0) && ((size_t)current < checklists.size())){
      return checklists[current]->do_processing(copilotOn);
    }
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
  if(dataref != NULL){
    dataref->reset_trig();
  }
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
        if(speech_active() && !dont_speak){
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
        if(checked || (copilotOn && (dataref != NULL) && dataref->trigered())){
          if(speech_active() && !dont_speak){
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
              say(text->c_str());
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
      //Look up if any of the sw_continue is matching 
      std::vector<std::pair<std::string, dataref_t *> >::iterator i;
      for(i = gotos.begin(); i != gotos.end(); ++i){
        //Finally - first time ever using the coma!
        if((i->second == NULL) || (i->second->reset_trig(), i->second->trigered())){
          //unconditional or dataref triggered
          continue_label = i->first;
          continue_flag = true;
          return true;
        }
      }
      continue_flag =  false;
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

  bool res = create_checklist(j, displaytext.c_str(), desc, width,
		              true_width_label, true_width_suffix,
			      index, force);
  delete [] desc;
  activate_next_item(true);
  return res;
}

const std::string& checklist::get_name()const
{
  return menutext;
}

const std::string& checklist::get_display_text()const
{
  return displaytext;
}

bool checklist_binder::check_references()
{
  int sizeOfAll = checklists.size();
  std::string nextLabel;
  std::map<std::string, int>::const_iterator ref;
  int tmp_w;
  for(int i = 0; i < sizeOfAll; ++i){
    checklists[i]->check_references(labels);
    tmp_w = checklists[i]->get_width();
    if(common_width < tmp_w){
      common_width = tmp_w;
    }
  }
  for(int i = 0; i < sizeOfAll; ++i) checklists[i]->set_width(common_width);

  return true;
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
  desc.text = text->c_str();
  desc.suffix = text1->c_str();
  desc.info_only = true;
  desc.item_void = true;
  desc.copilot_controlled = false;
  desc.c_text = text;
  desc.c_suffix = text1;
  return true;
}

bool remark_item::getDesc(checklist_item_desc_t &desc)
{
  desc.text = text->c_str();
  desc.suffix = (char *)"";
  desc.info_only = true;
  desc.item_void = true;
  desc.copilot_controlled = false;
  desc.c_text = text;
  desc.c_suffix = NULL;
  return true;
}

bool item_label::getDesc(checklist_item_desc_t &desc)
{
  desc.text = label->c_str();
  desc.c_text = label;
  if(!suffix->empty()){
    desc.suffix = suffix->c_str();
    desc.c_suffix = suffix;
  }else{
    desc.suffix = "CHECK";
    desc.c_suffix = NULL;
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

/*
 * Debug is a bitfield, with following meanings:
 *   - bit 0 - verbose parser output
 *   - bit 1 - verbosely evaluate expressions
 */

static int errors_found;

bool parse_clist(const std::string &fname, int debug)
{
  errors_found = 0;
  if((chklin=fopen(fname.c_str(), "r")) != NULL){
    parsed_file = strdup(fname.c_str());

    chkldebug       = debug & 1;
    debug_expr_eval = debug & 2;

    current_checklist = NULL;
    binder = NULL;
    int res = chklparse();
    fclose(chklin);
    free(parsed_file);
    parsed_file = NULL;
    chkllineno = 1;
    if((res == 0) && (errors_found == 0)){
        xcDebug("Xchecklist: Checklist read OK!\n");
        return true;
    }else{
      xcDebug("Xchecklist: %d errors found!\n", errors_found);
      return false;
    }
  }
  xcDebug("Xchecklist: Error encountered while reading Checklist!\n");
  return(false);
}

void chklerror(char const *s)
{
  xcErr("Xchecklist: %s in file %s, line %d near '%s'\n",
          s, parsed_file, chkllineno, chkltext);
  ++errors_found;
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
    *switchNext = continue_flag;
    return finished;
}

void checklist::add_continue_flag(std::string label, dataref_t *dref)
{
  gotos.push_back(std::pair<std::string, dataref_t *>(label, dref));
}


void checklist::check_references(const std::map<std::string, int> &labels)
{
  std::vector<std::pair<std::string, dataref_t *> >::iterator i;
  for(i = gotos.begin(); i != gotos.end(); ++i){
    if(!i->first.empty() && (labels.find(i->first) == labels.end())){
      xcWarn("Checklist '%s' contains reference to nonexistent checklist '%s'.\n", 
              displaytext.c_str(), i->first.c_str());
    }
  }
}

dataref_op::dataref_op(dataref_t *dr1, operation_t o, dataref_t *dr2) : dref1(dr1), dref2(dr2), op(o)
{
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
  dref1->registerDsc();
  dref2->registerDsc();
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

std::ostream& operator<<(std::ostream &output, const value& a)
{
  a.print(output);
  return output;
}

std::ostream& operator<<(std::ostream &output, const arith_op& a)
{
  output << *(a.value1) << " " << a.operation << " " << *(a.value2);
  return output;
}

std::ostream& operator<<(std::ostream &output, const procedure& a)
{
  output << a.name << "(";
  size_t s = a.params->size();
  for(size_t i = 0; i < s - 1; ++i){
    output << *((*a.params)[i]) << ", ";
  }
  output << *((*a.params)[s - 1]) << ")";
  return output;
}

std::string value::get_type_str()const
{
  switch(value_type){
    case TYPE_DOUBLE:
      return "(double)";
      break;
    case TYPE_FLOAT:
      return "(float)";
      break;
    case TYPE_INT:
      return "(int)";
      break;
    default:
      return "(!UNKNOWN!)";
      break;
  }
}


arith_op::arith_op(value *val1, char op, value *val2):value1(val1), value2(val2), operation(op)
{
}

bool arith_op::get_value(double &d)const
{
  double val1, val2;
  value1->get_value(val1);
  value2->get_value(val2);
  switch(operation){
    case '+':
      d = val1 + val2;
      cast(d);
      return true;
      break;
    case '-':
      d =  val1 - val2;
      cast(d);
      return true;
      break;
    case '*':
      d = val1 * val2;
      cast(d);
      return true;
      break;
    case '/':
      d = val1 / val2;
      cast(d);
      return true;
      break;
    case '^':
      d = pow(val1, val2);
      cast(d);
      return true;
      break;
    default:
      xcErr("Unsupprted operation '%c'.\n", operation);
      break;
  }
  return false;
}


bool procedure::get_value(double &d)const
{
  d = 0.0;
  if(!actor){
    return false;
  }
  bool res = actor(params, d);
  cast(d);
  return res;
}

static bool closer_than_func(const std::vector<value *> *params, double &res)
{
  if(params->size() != 3){
    xcErr("Function closer_than takes three parameters (value1, value2, epsilon)!\n");
    return false;
  }
  double val1, val2, eps;
  if(!(*params)[0]->get_value(val1)){
    xcErr("Error evaluating first parameter value!\n");
    return false;
  }
  if(!(*params)[1]->get_value(val2)){
    xcErr("Error evaluating second parameter value!\n");
    return false;
  }
  if(!(*params)[2]->get_value(eps)){
    xcErr("Error evaluating third parameter value!\n");
    return false;
  }
  //std::cout << "Closer than: " << val1 << " vs " << val2 << " < " << eps << std::endl;
  if(fabs(val1 - val2) < eps){
    res = 1.0;
  }else{
    res = 0.0;
  }
  return true;
}

static bool round_func(const std::vector<value *> *params, double &res)
{
  if(params->size() != 1){
    xcErr("Function round takes only one value!\n");
    return false;
  }
  double param_val;
  if(!(*params)[0]->get_value(param_val)){
    xcErr("Error evaluating parameter value!\n");
    return false;
  }
  res = round(param_val);
  return true;
}

static bool step_func(const std::vector<value *> *params, double &res)
{
  if(params->size() != 1){
    xcErr("Step function takes only one value!\n");
    return false;
  }
  double param_val;
  if(!(*params)[0]->get_value(param_val)){
    xcErr("Error evaluating parameter value!\n");
    return false;
  }
  if(param_val < 0){
    res = 0.0;
  }else{
    res = 1.0;
  }
  return true;
}

std::map<std::string, func_ptr_t> procedure::functions;

procedure::procedure(std::string n, std::vector<value *> *par):name(n), params(par), actor(NULL)
{
  if(functions.empty()){
    functions.insert(std::pair<std::string, func_ptr_t>("round", round_func));
    functions.insert(std::pair<std::string, func_ptr_t>("step", step_func));
    functions.insert(std::pair<std::string, func_ptr_t>("closer_than", closer_than_func));
  }
  std::map<std::string, func_ptr_t>::const_iterator i = functions.find(name);
  if(i != functions.end()){
    actor = i->second;
  }else{
    xcErr("Function %s not supported!\n", name.c_str());
  }
}

procedure::~procedure()
{
  if(params){
    for(std::vector<value *>::iterator i = params->begin(); i != params->end(); ++i){
      delete *i;
    }
  }
  delete params;
}

void value::cast(double &val)const
{ 
  if(value_type == TYPE_DOUBLE){
    //std::cout << "Cast to double" << std::endl;
    return;
  }
  if(value_type == TYPE_INT){
    //std::cout << "Cast to int" << std::endl;
    val = (int)val;
  }else{
    //std::cout << "Cast to float" << std::endl;
    val = (float)val;
  }
}

coloured_string::coloured_string(std::string str, std::string *colour)
{
  unsigned long idx = 0;
  if(colour != NULL){
    // have coloured string
    if(colour->size() > 0){
      //Have coloured string
      idx = p.get_colour_index(*colour);
    }else{
      idx = 1;
    }
  }
  cs.push_back(make_pair(str, idx));
  whole += str;
}


void coloured_string::resolve_colours()
{
  for(unsigned long i = 0; i < cs.size(); ++i){
    if(cs[i].second == 1){
      if(colour_stack.size() > 0){
        colour_stack.pop_back();
      }
      unsigned long tmp = colour_stack.size();
      if(tmp > 0){
        cs[i].second = colour_stack[tmp - 1];
      }
    }else{
      colour_stack.push_back(cs[i].second);
    }
  }
}

void coloured_string::append(coloured_string *str)
{
  std::vector<std::pair<std::string, unsigned long> >::const_iterator i;
  for(i = cs.begin(); i != cs.end(); ++i){
    str->append(i->first, i->second);
  }
}

void coloured_string::append(std::string str, unsigned long idx)
{
  cs.push_back(make_pair(str, idx));
  colour_stack.push_back(idx);
  whole += str;
}

const char *coloured_string::get_piece(int i, float *rgb)const
{
  if((i < 0) || ((unsigned long)i > cs.size())){
    return NULL;
  }
  p.get_colour(cs[i].second, rgb);
  return cs[i].first.c_str();
}

static float clamp_colour(const float c, const std::string name, const char *component)
{
  if(c < 0.0f){
    xcWarn("Component %s of colour %s is lower than zero.\n", component, name.c_str());
    return 0.0f;
  }
  if(c > 1.0f){
    xcWarn("Component %s of colour %s is higher than one.\n", component, name.c_str());
    return 1.0f;
  }
  return c;
}

palette::palette()
{
  t_rgb def;
  def.r = 152.0 / 255.0;
  def.g = 227.0 / 255.0;
  def.b = 192.0 / 255.0;
  colours.push_back(def); //0 - default
  colours.push_back(def); //1 - back through the stack
}

void palette::add_colour(const std::string name, const std::string r, const std::string g, const std::string b)
{
  t_rgb def;
  def.r = clamp_colour(fromString<float>(r), name, "r");
  def.g = clamp_colour(fromString<float>(g), name, "g");
  def.b = clamp_colour(fromString<float>(b), name, "b");
  colours.push_back(def);
  colour_names.insert(make_pair(name, colours.size() - 1));
}

unsigned long palette::get_colour_index(const std::string name)
{
  std::map<const std::string, unsigned long>::const_iterator i;
  i = colour_names.find(name);
  if(i == colour_names.end()){
    xcWarn("Unknown colour %s requested.\n", name.c_str());
    return 0;
  }
  return i->second;
}

void palette::get_colour(unsigned long index, float rgb[])
{
  if(index >= colours.size()){
    index = 0;
  }
  rgb[0] = colours[index].r;
  rgb[1] = colours[index].g;
  rgb[2] = colours[index].b;
}

