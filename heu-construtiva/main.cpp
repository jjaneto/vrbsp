#include "model.h"

bool operator<(const Solution &o1, const Solution &o2) {
  return o1.objective < o2.objective;
}

bool operator>(const Solution &o1, const Solution &o2) {
  return o1.objective > o2.objective;
}

Solution &Solution::operator=(const Solution &o1) {

  return *this;
}


inline void decideBest(Solution &f, const Solution &u, const Solution &v) {
  f = (u > v) ? u : v;
}

int objective_function() {
  
  return 0;
}

void updateChannels(const Solution &sol) {
  deque<Link> links = sol.getScheduledLinks();
  for (const Link &l : links) {
    chToLinks[l.ch].emplace_back(l.id);
  }
}

void split(Solution &dest, const Solution &src, int ch) {
  int ch1, ch2; //TODO
  deque<Link> links = src.getScheduledLinks();

  //--------------------------------------------------------
  // The two pairs of links with the largest interference
  double mxInter = -1.0; int idLink = -1;
  Link largest1, largest2;
  for (const Link &l : links) {
    if (l.interference > mxInter) {
      largest1 = l;
      mxInter = l.interference;
    }
  }

  mxInter = -1.0; idLink = -1;
  for (const Link &l : links) {
    if (l.interference > mxInter && l.id != largest1.id) {
      largest2 = l;
      mxInter = l.interference;
    }
  }

  Link Laux1(largest1), Laux2(largest2);
  Laux1.ch = ch1, Laux2.ch = ch2;
  //-------------------------------------------------------

  Solution prot = src;
  prot.insert(Laux1); prot.insert(Laux2);
  
  for (const Link &l : links) { //TODO: Choose the links RANDOMLY
    Solution prot_copy1 = prot, prot_copy2 = prot;
    //------------------
    // First, delete all links in channel c, as they will be split into c' and c''
    prot_copy1.deleteLinksFromChannel(ch);
    prot_copy2.deleteLinksFromChannel(ch);    
    //------------------

    
    if ((l.id == Laux1.id) || (l.id == Laux2.id))
      continue;

    //---------------
    // Put l in prot_copy1 and in prot_copy2 and compare which one has the better throughput
    prot_copy1.insert(l);
    prot_copy2.insert(l);
    //--------------
    decideBest(prot, prot_copy1, prot_copy2);
  }
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

double distance (double X_si, double Y_si, double X_ri, double Y_ri) {
  return hypot((X_si - X_ri), (Y_si - Y_ri));
}


void distanceAndInterference() {
  for (int i = 0; i < nConnections; i++) {
    double X_si = senders[i][X_c];
    double Y_si = senders[i][Y_c];

     
    for (int j = 0; j < nConnections; j++) {

      double X_rj = receivers[j][X_c];
      double Y_rj = receivers[j][Y_c];

      double dist = distance(X_si, Y_si, X_rj, Y_rj);

      distanceMatrix[i][j] = dist;       
      if (i == j) {
	interferenceMatrix[i][j] = 0.0;
      } else {
	interferenceMatrix[i][j] = (dist != 0.0) ? powerSender / pow(dist, alfa) : 1e8;	 
      }
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

  //TODO: Check why this is necessary
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

int main() {
  srand(time(NULL));
  //-----------------------
  // Read the file...
  readFile();
  // Initialize necessary things...
  /*
  chToLinks[25] = vector<int>();
  chToLinks[42] = vector<int>();
  chToLinks[43] = vector<int>();
  chToLinks[44] = vector<int>();
  chToLinks[45] = vector<int>();
  //-----------------------

  while (!links.empty()) {
    int idx = rand()%(links.size());
    int link = links[idx];

    Solution S_1, S_2;
    //-------------
    for (auto &el : chToLinks) {
      int ch = el.first;
      Link aux; aux.id = link, aux.ch = ch;
      S_1.insert(aux);
      if (whichBw(ch) > 20) {
	split(S_2, S_1, ch);
      }
      decideBest(S, S_1, S_2);
    }
    updateChannels(S);
    //-------------
    swap(links[idx], links.back());
    links.pop_back();
  }
  */
  return 0;
}