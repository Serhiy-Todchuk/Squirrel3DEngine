// TimeCounter.h: interface for the TimeCounter class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "types.h"
#include <vector>
#include <string>
#include "macros.h"

namespace Squirrel {

//TODO: rename to just "Time"
class SQCOMMON_API TimeCounter  
{
public:
	struct Node
	{
		Node(): time(0), acc(0), ms(0) {}
		Node(const std::string& n): time(0), acc(0), ms(0) {name = n;}
		std::string name;//id
		int time;//current node time (in miliseconds)
		float acc;//accumulated node time
		float ms;//mid node time
	};

private:
	TimeCounter();
	~TimeCounter();

private:
	std::vector<Node> mNodes;//
	float	mNewCount,mFrameCount,mLastCount;//working variables
	float	mFramesPerSecond;//fps count
	float	mDeltaTime;//time
	float	mTime;
	bool	mPause;

	static const int updateInterval = 1000;//in miliseconds

public:

	static TimeCounter& Instance();

	static uint32 GetTicks( );

	void calcTime();//calling in the end of frame
	inline float getFramesPerSecond()	{ return mFramesPerSecond; };//frames per second
	inline float getDeltaTime()			{ return mDeltaTime; }//time from start
	inline float getTime()				{ return mTime; }//time from start
	inline void pause(bool p)			{ mPause = p; }
	inline bool isPaused()				{ return mPause; }

	int		findNode	(const std::string& name)	{ for(unsigned i=0; i<mNodes.size(); i++) if(mNodes[i].name==name) return i; return -1; };
	int		addNode		(const Node& node)			{ mNodes.push_back(node); return int(mNodes.size()-1);};
	int		addNode		(const std::string& name)	{ return addNode(Node(name)); };
	void	delNode		(unsigned node)				{ if(node<mNodes.size()) mNodes.erase(mNodes.begin() + node);};
	Node *	getNode		(unsigned node)				{ return (node < mNodes.size()) ? &mNodes[node] : NULL; };
	int		getNodesNum	()							{ return (int)mNodes.size();};

	void	setNodeTimeBegin(unsigned node);
	void	setNodeTimeEnd	(unsigned node);
};

}//namespace Squirrel {
