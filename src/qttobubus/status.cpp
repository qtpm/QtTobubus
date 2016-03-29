#include "status.h"

static int statusid = 0;

Call::Call(QObject* parent)
	: QObject(parent)
{
}

void Call::RegisterStatus()
{
    if (statusid == 0) {
        statusid = qRegisterMetaType<Call::Status>("Call::Status");
    }
}

#include "moc_status.cpp"
