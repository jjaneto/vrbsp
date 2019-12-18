#include "model.h"

bool operator<(const Solution &o1, const Solution &o2) {
  return o1.objective < o2.objective;
}

bool operator>(const Solution &o1, const Solution &o2) {
  return o1.objective > o2.objective;
}

Solution &Solution::operator=(const Solution &o1) {

}


int objective_function() {
  
  return 0;
}

void updateChannels(const Solution &sol) {

}

void split(Solution &dest, const Solution &src, int ch) {

}

void decideBest(Solution &f, const Solution &u, const Solution &v) {
  f = (u > v) ? u : v;
}

void Solution::insert(const Solution &S, int ch, int link) {

}

int whichBw(int ch) {
  if (ch >= 26 && ch <= 37)
    return 40;
  else if (ch >= 38  && ch <= 43)
    return 80;
  else if (ch == 44 || ch == 45)
    return 160;
  
  return 20;
}

int main() {
  srand(time(NULL));
  //-----------------------
  //scanf("%d", &L);
  L = 0;

  for (int i = 1; i <= L; i++) {
    links.emplace_back(i);
  }

  while (!links.empty()) {
    int idx = rand()%(links.size());
    int link = links[idx];

    Solution S_o, S_t;
    //-------------
    for (int ch : channels) {
      S_o.insert(S, ch, link);
      if (whichBw(ch) > 20) {
	split(S_t, S_o, ch);
      }
      decideBest(S, S_o, S_t);
    }
    updateChannels(S);
    //-------------
    swap(links[idx], links.back());
    links.pop_back();
  }
  return 0;
}
