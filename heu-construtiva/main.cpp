#include "model.h"

bool operator<(const Solution &o1, const Solution &o2) {
  return o1.objective < o2.objective;
}

bool operator>(const Solution &o1, const Solution &o2) {
  return o1.objective > o2.objective;
}

bool operator==(const Solution &o1, const Solution &o2) {

  if (o1.objective != o2.objective)
    return false;

  if (o1.scheduled_links.size() != o2.scheduled_links.size())
    return false;

  bool cond = true;

  const deque<Link> arr1 = o1.getScheduledLinks();
  const deque<Link> arr2 = o2.getScheduledLinks();

  for (int i = 0; i < int(arr1.size()) && cond; i++) {
    bool go = false;
    for (int j = 0; j < int(arr2.size()); j++) {
      if (arr1[i] == arr2[j]) {
        go = true;
      }
    }

    cond &= go;
  }
  
  return cond;
}

bool operator==(const Link &o1, const Link &o2) {
  if (o1._idR != o2._idR) {
    //printf("%d %d\n", o1._idR, o2._idR);
    return false;
  }

  if (o1.id != o2.id) {
    //puts("eeeeeeeeeeeeeeeeeeeeeeeeeeeee");
    return false;
  }

  if (o1.ch != o2.ch) {
    //puts("uuuuuuuuuuuuuuu");
    return false;
  }

  if (o1.bw != o2.bw) {
    //puts("iiiiiiiiiiiiiiiiiii");
    return false;
  }

  if (o1.interference != o2.interference) {
    //puts("KKKK");
    return false;
  }
  
  if (o1.SINR != o2.SINR) {
    //puts("qqqqqqq");
    return false;
  }

  if (o1.MCS != o2.MCS) {
    //puts("dont know");
    return false;
  }

  return true;
}

Solution &Solution::operator=(const Solution &o1) {
  this->objective = o1.objective;
  this->scheduled_links = o1.scheduled_links;
  return *this;
}

inline void decideBest(Solution &f, const Solution &u, const Solution &v) {
  if (u > f || v > f) {
    if (u > v) {
      printf("best solution is S1 with objective %.2lf\n", u.getObjective());
    } else if(v > u) {
      printf("DIVIDIR FO MELHOR S2 with objective %.2lf\n", v.getObjective());
    }
    f = (u > v) ? u : v;
  } else {
    
  }
}

void updateChannels(const Solution &sol) {
  deque<Link> links = sol.getScheduledLinks();
  for (const Link &l : links) {
    chToLinks[l.ch].emplace_back(l.id);
  }
}

void split(Solution &dest, const Solution &src, int ch) {
  int ch1 = mapChtoCh[ch].first, ch2 = mapChtoCh[ch].second;
  printf("split: ch1 %d ch2 %d\n", ch1, ch2);
  deque<Link> links = src.getLinksInChannel(ch);

  if (links.size() < 2) {
    printf("nao ha conexoes suficientes no canal %d\n", ch);
    return;
  }

  Link largest1;
  for (const Link &l : links) {
    if (l.interference > largest1.interference) {
      //printf("?:: link %d\n", l.id);
      largest1 = l;
    }
  }

  Link largest2;
  for (const Link &l : links) {
    if ((l.interference > largest2.interference) && (l.interference != largest1.interference)) {
      //printf("ahaahahahah link %d\n", l.id);
      largest2 = l;
    }
  }

  auto it = links.begin();
  while (it != links.end()) {
    //printf("visitando link %d (equal %d)\n", it->id, (*it == largest2));
    if ((*it == largest1) || (*it == largest2)) {
      //puts("passei");
      it = links.erase(it);
    } else {
      it++;
    }
  }

  Solution current(src);
  current.clearChannel(ch);

  printf("apos apagar tem %lu links (array links %lu)\n", current.getScheduledLinks().size(), links.size());
  largest1.setChannel(ch1);
  largest2.setChannel(ch2);
  current.insert(largest1);
  current.insert(largest2);

  for (const Link &laux : links) { //TODO: choose the links RANDOMLY
    //TODO: Be careful with the repetition of largest1 and largest2
    printf("     for loop\n");
    Solution copy1(current), copy2(current);

    Link lcp1(laux), lcp2(laux);
    lcp1.setChannel(ch1), lcp2.setChannel(ch2);

    copy1.insert(lcp1);
    copy2.insert(lcp2);

    current = (copy1 > copy2) ? copy1 : copy2;
  }

  dest = current;

  printf(" OBJECTIVE OF DEST IS %.2lf\n", dest.getObjective());
  
}

double distance (double X_si, double Y_si, double X_ri, double Y_ri) {
  return hypot((X_si - X_ri), (Y_si - Y_ri));
}


void distanceAndInterference() {
  for (int i = 0; i < nConnections; i++) {
    double X_si = receivers[i][X_c];
    double Y_si = receivers[i][Y_c];

     
    for (int j = 0; j < nConnections; j++) {

      double X_rj = senders[j][X_c];
      double Y_rj = senders[j][Y_c];

      double dist = distance(X_si, Y_si, X_rj, Y_rj);

      distanceMatrix[i][j] = dist;

      double value = (dist != 0.0) ? powerSender / pow(dist, alfa) : 1e9;
      interferenceMatrix[i][j] = value;
    }
  }
}

double convertDBMToMW(double _value) {
  double result = 0.0, b;

  b = _value / 10.0;// dBm dividido por 10
  result = pow(10.0, b);//Converte de DBm para mW

  return result;
}

void convertTableToMW(const vector<vector<double> > &_SINR, vector<vector<double> > &_SINR_Mw) {
  double result, b;
  for (int i = 0; i < _SINR_Mw.size(); i++) {
    for (int j = 0; j < _SINR_Mw[i].size(); j++) {

      if (_SINR[i][j] != 0) {
        b = _SINR[i][j] / 10.0;// dBm divided by 10
        result = pow(10.0, b);//Convert DBM to mW

        _SINR_Mw[i][j] = result;
      } else {
        _SINR_Mw[i][j] = 0;
      }
    }
  }
}

void readFile() {
  SINR.assign(10, vector<double>(4, 0));
  double aux1;
  scanf("%lf", &aux1);
  scanf("%d %lf %lf %lf %lf %lf %lf %lf %lf", &nConnections, &ttm, &alfa, &noise, &powerSender, &aux1, &aux1,
	&aux1, &aux1);

  if (noise != 0) {
    noise = convertDBMToMW(noise);
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      scanf("%lf", &dataRates[i][j]);
    }
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      scanf("%lf", &SINR[i][j]);
    }
  }

  convertTableToMW(SINR, SINR);

  for (int i = 0; i < nConnections; i++) {
    double x, y; scanf("%lf %lf", &x, &y);
    receivers[i][X_c] = x;
    receivers[i][Y_c] = y;
  }

  for (int i = 0; i < nConnections; i++) {
    double x, y; scanf("%lf %lf", &x, &y);
    senders[i][X_c] = x;
    senders[i][Y_c] = y;
  }

  memset(interferenceMatrix, 0, sizeof interferenceMatrix);
  memset(distanceMatrix, 0, sizeof distanceMatrix);

  distanceAndInterference();
}

inline void mapSplitChannels() {
  mapChtoCh[44] = {38, 39};
  mapChtoCh[38] = {26, 27};
  mapChtoCh[39] = {28, 29};
  mapChtoCh[26] = {1, 2};
  mapChtoCh[27] = {3, 4};
  mapChtoCh[28] = {5, 6};
  mapChtoCh[29] = {7, 8};
  mapChtoCh[45] = {40, 41};
  mapChtoCh[40] = {30, 31};
  mapChtoCh[41] = {32, 33};
  mapChtoCh[30] = {9, 10};
  mapChtoCh[31] = {11, 12};
  mapChtoCh[32] = {13, 14};
  mapChtoCh[33] = {15, 16};
  mapChtoCh[42] = {34, 35};
  mapChtoCh[34] = {17, 18};
  mapChtoCh[35] = {19, 20};
  mapChtoCh[43] = {36, 37};
  mapChtoCh[36] = {21, 22};
  mapChtoCh[37] = {23, 24};    
}

void printSolution(const Solution &check) {
  printf("Solution is: objective %.2lf\n", check.getObjective());
  deque<Link> aux = check.getScheduledLinks();

  for (const Link &l : aux) {
    printf("LINK id %d\n", l.id);
    printf("   _idR %d, _idS %d, ch %d, bw %d, interference %.3lf, SINR %.3lf, MCS %d\n", l._idR, l._idS, l.ch, l.bw, l.interference, l.SINR, l.MCS);
  }
}

int main() {
  srand(time(NULL));
  //-----------------------
  // Read the file...
  readFile();
  // Initialize necessary things...
  chToLinks[25] = vector<int>();
  chToLinks[42] = vector<int>();
  chToLinks[43] = vector<int>();
  chToLinks[44] = vector<int>();
  chToLinks[45] = vector<int>();
  mapSplitChannels();
  //-----------------------

  for (int i = 0; i < nConnections; i++)
    links.emplace_back(i);

  int cont = 0;
  while (!links.empty()) {
    int idx = rand()%(links.size());
    int link = links[idx];
    printf("link %d\n", link);
    //-------------
    Solution Scopy(S);
    for (auto &el : chToLinks) {
      printf("--------------> visitando channel %d\n", el.first);
      Solution S1(S), S2;
      int ch = el.first;
      Link aux(link);
      aux.setChannel(ch);
      printf("    inserindo link %d (links atuais %lu)\n", aux.id, S1.getScheduledLinks().size());
      S1.insert(aux);
      //
      if (whichBw(ch) > 20) {
        split(S2, S1, ch);
      }
      
      //
      decideBest(Scopy, S1, S2);
      //
    }
    if (Scopy > S) {
      S = Scopy;
    }

    printSolution(S);
    //-------------
    swap(links[idx], links.back());
    links.pop_back();
    puts("\n\n");
    if (++cont == 4)
      break; //TODO
  }
  return 0;
}
