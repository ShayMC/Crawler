
#include "HNode.h"

HNode::HNode()
{
}

HNode::HNode(string noRobotUrls) : _noRobotUrls(noRobotUrls), _hostBusy(0)
{
}

void HNode::setNoRobotUrls(string noRobotUrls)
{
        _noRobotUrls = noRobotUrls;
}


string HNode::getNoRobotUrls()
{
        return _noRobotUrls;
}

void HNode::setHostBusy()
{
        _hostBusy = 1;
}

void HNode::setHostNotBusy()
{
        _hostBusy = 0;
}

bool HNode::isHostBusy()
{
        return _hostBusy;
}
