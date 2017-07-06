
#ifndef HNODE_H
#define HNODE_H

#include <string>

using namespace std;

class HNode
{
public:
HNode();
HNode(string noRobotUrls);

void setNoRobotUrls(string noRobotUrls);
void setHostBusy();
void setHostNotBusy();

string getNoRobotUrls();
bool isHostBusy();

private:
string _noRobotUrls;
char _hostBusy;
};

#endif
