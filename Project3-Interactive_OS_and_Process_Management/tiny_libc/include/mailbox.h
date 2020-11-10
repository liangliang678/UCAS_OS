#ifndef INCLUDE_MAIL_BOX_U_
#define INCLUDE_MAIL_BOX_U_

int mbox_open(char *);
void mbox_close(int);
void mbox_send(int, void *, int);
void mbox_recv(int, void *, int);

#endif