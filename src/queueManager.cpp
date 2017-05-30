#include "queueManager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "exceptions.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <resolv.h>
#include "SMTPServer.h"

#define MAX_MX_RECORDS_NUM 10

QueueManager::QueueManager(char* _path, char* _server_name, MailboxManager& _manager):
	counter(0), mailboxes(_manager)
{
	path = strdup(_path);
	server_name = strdup(_server_name);
}

QueueManager::~QueueManager()
{
	free(path);
	free(server_name);

	if (server) {
		server->QueueManagerIsDead();
	}
}

void QueueManager::SMTPServerIsDead()
{
	server = NULL;
}

void QueueManager::CreateMailQueueDir()
{
	int res = mkdir(path, 0700);
	if (res == -1) {
		if (errno == EEXIST)
			puts("CreateMailQueueDir: already exists");
		else {
			puts("Could not create directory for Mail Queue");
			throw FatalException();
		}
	}
}

char** QueueManager::GetMxRecords(char* host)
{
	char** mx_records = new char*[MAX_MX_RECORDS_NUM];
	mx_records[0] = NULL;

	u_char nsbuf[1024];
    ns_msg msg;
    ns_rr rr;
    int l;

	l = res_search(host, ns_c_any, ns_t_mx, nsbuf, sizeof(nsbuf));
    if (l < 0) {
      perror(host);
    } else {
      ns_initparse(nsbuf, l, &msg);
      l = ns_msg_count(msg, ns_s_an);
      for (int i = 0; i < l; i++) {
        if (ns_parserr(&msg, ns_s_an, i, &rr)) {
            /* WARN: ns_parserr failed */
            continue;
        }
        if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == ns_t_mx) {
            char mxname[128];
            dn_expand(ns_msg_base(msg), ns_msg_base(msg) + ns_msg_size(msg), ns_rr_rdata(rr) + NS_INT16SZ, mxname, sizeof(mxname));
            int k = 0;
            while (mx_records[k]) {
            	++k;
            }
            mx_records[k] = strdup(mxname);
            mx_records[k+1] = NULL;
            //u_int priority = ns_get16(ns_rr_rdata(rr));
            //printf("%s\n", mxname);
        }
      }
    }
	return mx_records;
}

void QueueManager::DeleteMessage(char* name)
{

}

bool QueueManager::OpenEnvAndMsgFiles(char* id, int& env, int& msg)
{
	char buf[1024];

	snprintf(buf, sizeof(buf), "%s%s.env", path, id);
	env = open(buf, O_RDONLY);

	snprintf(buf, sizeof(buf), "%s%s.msg", path, id);
	msg = open(buf, O_RDONLY);

	if (env == -1 || msg == -1) {
		char err_msg[1024];
		snprintf(err_msg, sizeof(err_msg), "Queue manager can't open files with message id: %s", id);
		throw err_msg;
		close(env);
		close(msg);
		return false;
	}

	return true;
}

bool QueueManager::DeliverToTheMailbox(char* id)
{
	return true;
}

bool QueueManager::ForwardMessage(char* id, char* str, int fd_msg)
{
	char *left = str, *right = NULL, sender[64], rcpt[64];
	right = strchr(str, ' ');
	memcpy(sender, left, right-left);
	sender[right-left] = '\0';
	left = ++right;
	right = strchr(left, '\n');
	memcpy(rcpt, left, right-left);
	rcpt[right-left] = '\0';

	left = strchr(rcpt, '@');
	left++;
	char** mx_rec = GetMxRecords(left);
	server->NewClientSession(mx_rec[0], sender, rcpt, fd_msg);
	int i = 0;
	while(mx_rec[i]) {
		free(mx_rec[i]);
		i++;
	}
	delete[] mx_rec;
	return true;
}

void QueueManager::ProcessSingleMessage(char* id)
{
	int env = 0, msg = 0;
	if (false == OpenEnvAndMsgFiles(id, env, msg))
		return;

	char buf[1024];

    char *left, *right;
    int n = read(env, buf, sizeof(buf)-1);
    buf[n] = '\0';

    char* end = NULL;
    long time = strtol(buf, &end, 10);
    if ((end == buf) || (errno == ERANGE) || (time <= 0)) {
    	throw("QueueManager: error parsing env, time is invalid\n");
    }

    bool failed;
    left = strchr(buf, '\n');
    if (!left) {
    	throw("Error when parsing env file");
    }
    left++;
    right = strchr(left, ' ');
    if (!right) {
    	throw("Error when parsing env file");
    }
    if (0 == strncmp(left, "failed", right-left)) {
    	failed = true;
    }
    else if (0 == strncmp(left, "new", right-left)) {
    	failed = false;
    } else {
    	throw("Error when parsing env file");
    }
    left = right+1;

    //analyze option; if forward call ForwardMessage, if deliver call DeliverToTheMailbox
    right = strchr(left, ' ');
    if (!right) {
    	throw("Error when parsing env file");
    }
    if (0 == strncmp(left, "forward", right - left)) {
    	ForwardMessage(id, right+1, msg);
    } else if (0 == strncmp(left, "deliver", right - left)) {
    	DeliverToTheMailbox(id);
    } else {
    	throw("Unknown option");
    }

	close(env);
}

void QueueManager::ProcessQueue()
{

}