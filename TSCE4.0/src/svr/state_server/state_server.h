/********************************************************************
��Ȩ����:�˳���������������Ϣ�������޹�˾
��Ŀ����:�������ײ���ƽ̨
��    ��:V1.0
����ϵͳ:Red Hat Enterprise Linux Server release 5.5
�ļ�����:listIP.h
�ļ�����:�����������iP����̬���IP���������IP�ͽڵ�״̬��ɾ��ָ��IP�Ⱥ���
�� Ŀ ��:�����ܷ�������Ʒ��
�� �� Ա:�ι���
����Ŀ¼��/root/dgd/c_test/ifnet/listIP.h
��������:2013��01��18��
��    ��:
�޸�����:2013��03��7�� ������ͨ�����ݿ����ӣ�������ʼ�����в�ѯ���޸ģ�ɾ��
*********************************************************************/

#define STR_SIZE 16 //ip��ַ
#define MAX_IP_COUNT 7

#define MAX_FDS                 1024
#define MAX_EVENTS              1024
#define BACKLOG                 1024 
 /**
 �ڵ�����ݽṹ
 num ��ʾ�ڵ��״̬��״̬��Ϊ��0:������� 1:�����Ϣ��ȡ����Ҳ��ʾ����û������ 2:�����ػ� 3:���粻ͨ
 str ��ʾ�ڵ�IP
 */
 struct node
{
    char ip[16];
    int fd;
    int flag;
    struct node *next;
};

 /**
 ����ָ��״̬�Ľڵ�IP
 */
 struct node *insert (int sockfd, int flag);
 
 /**
 ɾ��ָ��IP�Ľڵ�
 */
 struct node *delet ();
 
 /**
 ����ڵ���Ϣ
 */
 void printhead ();
 int pingcommand (char *ipset);
 
void* thread_ns (void* argv);
