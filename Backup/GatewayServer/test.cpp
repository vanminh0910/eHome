#include <string.h>
#include "list.h"

typedef struct {
    char topic[64];
    char message[64];
    int sentAt;
}MQTTMessage;

int main(int argc, char** argv) {
    List<MQTTMessage> *list = new List<MQTTMessage>();
    printf("\nAdding item 1");
    MQTTMessage m1;
    sprintf(m1.topic, "topic 1");
    sprintf(m1.message, "message 1");
    m1.sentAt = 10;
    list->push_back(m1);
    printf("\nAdding item 2");
    MQTTMessage m2;
    sprintf(m2.topic, "topic 2");
    sprintf(m2.message, "message 2");
    m2.sentAt = 200;
    list->push_back(m2);
	MQTTMessage m3;
	printf("\nAdding item 3");
    sprintf(m3.topic, "topic 3");
    sprintf(m3.message, "message 3");
    m3.sentAt = 3000;
	list->push_back(m3);
	fflush(stdout);
	//printf("\nRemoving item 1");
	//list->removeNode(list->tail);
	
	//list->removeNode(list->head);
	Node<MQTTMessage>* mqtt = list->head;
	while (mqtt != NULL) {
		if (strcmp(((MQTTMessage)mqtt->value).topic, "topic 2") == 0) {
			printf("\nRemoving item:");
			printf("\nItem: %s\n", ((MQTTMessage)mqtt->value).topic);
			list->removeNode(mqtt);
			mqtt = list->head;
		} else
			mqtt = mqtt->next;
	}
	
	while (!list->empty()) {
		Node<MQTTMessage>* n = list->pop_back();
		printf("\nItem: %s\n", ((MQTTMessage)n->value).topic);
		delete n;
	}
    return 0;
}



