#include "Notification.h"

namespace Squirrel {

ObjectNotify::ObjectNotify()
{

}

ObjectNotify::~ObjectNotify()
{
	delFromRecipients();
}

void ObjectNotify::addToRecipients(const std::string& notification)
{
	NotificationCenter::Instance().addRecipient(notification, this);	
}

void ObjectNotify::delFromRecipients()
{
	NotificationCenter::Instance().deleteRecipient(this);
}


NotificationCenter::NotificationCenter()
{

}

NotificationCenter::~NotificationCenter()
{
	recipients.clear();
}

void NotificationCenter::deleteRecipient(ObjectNotify * recipient)
{
	if(!recipient) return;

	std::map<std::string, ObjectNotifyPtrArr>::iterator it = recipients.begin();

	while(it != recipients.end ())
	{
		for(int i = 0; i < (int)it->second.size(); ++i) 
			if(it->second[i])
				if(it->second[i]==recipient)
					it->second.erase( it->second.begin() + i );
		it++;
	}
}

void NotificationCenter::addRecipient(const std::string& notification, ObjectNotify * recipient)
{
	std::map<std::string, ObjectNotifyPtrArr>::iterator it = recipients.find ( notification );

	if(it==recipients.end())
	{
		recipients[notification] = ObjectNotifyPtrArr();
		it = recipients.find ( notification );
	}

	it->second.push_back(recipient);

}
void NotificationCenter::notify(const std::string& notification)
{
	std::map<std::string, ObjectNotifyPtrArr>::iterator it = recipients.find ( notification );

	while(it != recipients.end ())
	{
		for(int i = 0; i < (int)it->second.size(); ++i) 
			if(it->second[i])
				it->second[i]->notify(notification);
		it++;
	}
}

NotificationCenter& NotificationCenter::Instance()
{
	static NotificationCenter instance;
	return instance;
}

}//namespace Squirrel {