#pragma once

#include <string>
#include <vector>
#include <map>
#include "macros.h"

namespace Squirrel {

class SQCOMMON_API ObjectNotify
{
public:

	ObjectNotify(void);
	virtual ~ObjectNotify(void);

	void addToRecipients(const std::string& notification);
	void delFromRecipients();

	virtual void notify(const std::string& notification) = 0;

};

//OPTIMIZE: list
typedef std::vector<ObjectNotify *> ObjectNotifyPtrArr;

class SQCOMMON_API NotificationCenter
{
	friend class ObjectNotify;

private:

	std::map<std::string, ObjectNotifyPtrArr> recipients;

	void deleteRecipient(ObjectNotify * recipient);

	NotificationCenter(void);

public:

	virtual ~NotificationCenter(void);

	void addRecipient(const std::string& notification, ObjectNotify * recipient);

	void notify(const std::string& notification);

	static NotificationCenter& Instance();

};

}//namespace Squirrel {