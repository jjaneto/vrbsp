#include "../heu-construtiva/model.h"

using namespace std;

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
    return false;
  }

  if (o1.id != o2.id) {
    return false;
  }

  if (o1.ch != o2.ch) {
    return false;
  }

  if (o1.bw != o2.bw) {
    return false;
  }

  if (o1.interference != o2.interference) {
    return false;
  }
  
  if (o1.SINR != o2.SINR) {
    return false;
  }

  if (o1.MCS != o2.MCS) {
    return false;
  }

  if (o1.distanceSenderReceiver != o2.distanceSenderReceiver) {
    return false;
  }

  return true;
}

Solution &Solution::operator=(const Solution &o1) {
  this->objective = o1.objective;
  this->scheduled_links = o1.scheduled_links;
  return *this;
}


int main() {
  // O formato do arquivo de entrada eh [link-id canal]
  vector<Link> links(nConnections, Link());

  for (int i = 0; i < nConnections; i++) {
    int id, channel;
    scanf("%d %d\n", &id, &channel);
    links[id].setChannel(channel);
  }

  Solution F;
  F.insertLinks(links);

  
  return 0;
}
