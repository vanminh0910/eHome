#ifndef __LIST_H__
#define __LIST_H__
#endif

#include <stdio.h>
#include <cstdlib>
#include <assert.h>

template < typename T > struct Node;

// declare List. the compiler needs to know that it is a template
template < typename T > struct List
{
    public:     
        // default constructor - constructs an empty list
        List();

        inline ~List() {
            printf("\nDestroying list");
            assert_invariant();
            while( !empty() )
                delete pop_back() ; 
        }

        bool valid() const;

        // insert (copy of) object at the back of the list
        void push_back( const T object ) ;

        // remove object at the back of the list
        Node<T>* pop_back() ;
         
        // remove node from the list
        void removeNode( const Node<T>* node ) ;

        inline int size() const { return sz ; }
        inline bool empty() const { return (size() == 0); }
        inline void assert_invariant() const { assert( valid() ) ; }

    //private:
        Node<T>* head ;
        Node<T>* tail ;
        std::size_t sz ; // number of nodes
};


// node holds a *value* - a copy of (not a pointer to) an object
template < typename T > struct Node
{
    public:
        Node(const T v, Node* next_node = NULL, Node* prev_node = NULL);
        ~Node();
        T value ;
        Node* next ;
        Node* prev ;

    friend class List<T> ;
};

template < typename T > Node<T>::Node( const T v, Node* next_node, Node* prev_node) {
    value = v;
    next = next_node;
    prev = prev_node;
}

template < typename T > Node<T>::~Node() {
    //printf("\nDestroying node");
}

template <typename T> List<T>::List()
{
     head = tail = NULL;
     sz = 0;
     assert_invariant();
}

template < typename T > bool List<T>::valid() const
{
	return true;
    if( sz == 0 ) {
        //printf("\nSize is 0 check");
        return head==NULL && tail==NULL ;
    
    }else if( sz == 1 ) {
        //printf("\nSize is 1 check");
        return head!=NULL && tail==head && head->prev==NULL && head->next==NULL ;

    } else  {
        //printf("\nSize is >1 check");
        if( head!=NULL && tail!=NULL && head!=tail
            && head->prev==NULL && tail->next==NULL )
        {
            //printf("\nSize is >1 and good check");
            std::size_t n1 = 0 ;
            for( Node<T>* t = head ; t != NULL ; t = t->next ) ++n1 ;

            std::size_t n2 = 0 ;
            for( Node<T>* t = tail ; t != NULL ; t = t->prev ) ++n2 ;

            return n1==sz && n1==n2 ;
        }
        else return false ;
    }
}

template < typename T > void List<T>::push_back( const T object )
{
    assert_invariant();

    if( empty() ) {
        //printf("\nList is empty. Adding item.");
        head = tail = new Node<T>(object) ;
    } else {
        //printf("\nList is not empty");
        tail->next = new Node<T>( object, NULL, tail );
        tail = tail->next ; 
    }
    ++sz ;
    assert_invariant() ;
}

template < typename T > void List<T>::removeNode( const Node<T>* node )
{
    //printf("\nNode to remove: %p", node);
	//printf("\nHead Node: %p", head);
	//printf("\nTail Node: %p", tail);
    assert_invariant();

    if( empty() || node == NULL) {
        //printf("\nList is empty or node to remove is null. No node to remove");
        return;
    } else {
        Node<T> *pPre = NULL, *pDel = NULL;
    
        /* Check whether it is the head node?
         if it is, delete and update the head node */
        if (head == node) {
            //printf("\nDeleted node is head");
            /* point to the node to be deleted */
            pDel = head;
            if (head == tail) {
				//printf("\nDeleted node is also tail");
				fflush(stdout);
                head = tail = NULL;
			} else {
				/* update */
				head = pDel->next;
				head->prev = NULL;
			}
            delete pDel;
            --sz ;
            assert_invariant() ;
            return;
        }
        
        pPre = head;
        pDel = head->next;
        
        /* traverse the list and check the value of each node */
        while (pDel != NULL) {
            if (pDel == node) {
				/* If it is the last node, update the tail */
                if (pDel == tail) {
                    //printf("\nDeleted node is tail");
                    tail = pPre;
                } else {
					/* Update the list */
					pPre->next = pDel->next;
					(pDel->next)->prev = pPre;
				}
                delete pDel; /* Here only remove the first node with the given value */
                --sz ;
                assert_invariant() ;
                return; /* break and return */
            }
            pPre = pDel;
            pDel = pDel->next;
        }
    }
}

template < typename T > Node<T>* List<T>::pop_back()
{
	//printf("\nPop back");
    Node<T>* node = NULL;
    assert( !empty() ) ;
    assert_invariant() ;

    if( size() == 1 ) {
		//printf("\nPop back when size is 1");
        node = head;
        head = tail = NULL ;
    } else {
		//printf("\nPop back when size is %d", sz);
        node = tail;
        tail = tail->prev ;
        tail->next = NULL ; 
    }

    --sz ;

    assert_invariant() ;
	//printf("\nPop back done");
    return node;
}

/* Example code
 *

typedef struct {
    char topic[64];
    char message[64];
    int sentAt;
}MQTTMessage;

int main(int argc, char** argv) {
    List<MQTTMessage> *list = new List<MQTTMessage>();
    printf("\n0\n");
    MQTTMessage m1;
    sprintf(m1.topic, "topic 1");
    sprintf(m1.message, "message 1");
    m1.sentAt = 10;
    list->push_back(m1);
    printf("\n1\n");
    MQTTMessage m2;
    sprintf(m2.topic, "topic 2");
    sprintf(m2.message, "message 2");
    m2.sentAt = 200;
    list->push_back(m2);
    printf("\n2\n");
    Node<MQTTMessage>* n1 = list->pop_back();
    printf("\nItem 1: %s\n", ((MQTTMessage)n1->value).topic);
    Node<MQTTMessage>* n2 = list->pop_back();
    printf("\nItem 2: %s\n", ((MQTTMessage)n2->value).topic);
    delete n1;
    delete n2;
    return 0;
}
*/



