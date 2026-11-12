#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <cctype>
namespace pimorph {
struct ImportBrowser {
 struct Entry{std::string name,path;bool directory;};
 std::mutex mutex;std::string path,requested;std::vector<Entry>entries;int page=0;bool truncated=false;std::atomic<bool>scan{false},dirty{false};
 void navigate(const std::string& p){if(p.empty()||p.size()>1024||p[0]!='/')return;std::unique_lock<std::mutex>l(mutex,std::try_to_lock);if(!l)return;requested=p;scan=true;}
 void reopen(){std::unique_lock<std::mutex>l(mutex,std::try_to_lock);if(!l)return;requested=path.empty()?"/media":path;scan=true;}
 void work(){if(!scan.exchange(false))return;std::string next;{std::lock_guard<std::mutex>l(mutex);next=requested;}
  std::vector<Entry> list;std::error_code ec;bool overflow=false;size_t visited=0;
  for(std::filesystem::directory_iterator it(next,ec),end;!ec&&it!=end;it.increment(ec)){
   if(++visited>8192){overflow=true;break;}auto name=it->path().filename().string();if(name.empty()||name[0]=='.')continue;
   bool dir=it->is_directory(ec);if(ec){ec.clear();continue;}std::string ext=it->path().extension().string();std::transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char c){return std::tolower(c);});
   if(!dir&&(!it->is_regular_file(ec)||ext!=".wav"))continue;if(list.size()>=512){overflow=true;continue;}
   auto full=it->path().string();if(full.size()<=1024)list.push_back({name,full,dir});
  }
  std::sort(list.begin(),list.end(),[](const Entry&a,const Entry&b){return a.directory!=b.directory?a.directory>b.directory:a.name<b.name;});
  std::lock_guard<std::mutex>l(mutex);if(next!=requested)return;path=next;entries=std::move(list);page=0;truncated=overflow;dirty=true;
 }
 std::string read(const std::string& key){std::unique_lock<std::mutex>l(mutex,std::try_to_lock);if(!l)return "Loading...";
  if(key=="browse_path")return path.size()>110?"..."+path.substr(path.size()-107):path;
  if(key=="browse_page")return std::to_string(page+1)+" / "+std::to_string(std::max(1,int((entries.size()+5)/6)))+(truncated?" · first 512":"");
  int i=key.empty()?-1:key.back()-'0';if(i>=0&&i<6&&page*6+i<int(entries.size()))return (entries[page*6+i].directory?"[DIR] ":"")+entries[page*6+i].name;return "—";
 }
 std::string click(int row){if(scan.load())return {};std::unique_lock<std::mutex>l(mutex,std::try_to_lock);if(!l)return {};int i=page*6+row;if(i<0||i>=int(entries.size()))return {};auto e=entries[i];l.unlock();if(e.directory){navigate(e.path);return {};}return e.path;}
 void step(int n){std::unique_lock<std::mutex>l(mutex,std::try_to_lock);if(!l)return;page=std::clamp(page+n,0,std::max(0,int((entries.size()+5)/6)-1));dirty=true;}
 void up(){std::unique_lock<std::mutex>l(mutex,std::try_to_lock);if(!l)return;auto p=std::filesystem::path(path).parent_path().string();l.unlock();navigate(p.empty()?"/":p);}
};
}
