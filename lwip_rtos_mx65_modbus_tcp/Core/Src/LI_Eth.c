/*
 * LI_Eth.c
 *
 *  Created on: Apr 8, 2026
 *      Author: Joselito Junior
 *
 *  Ethernet transport layer.
 *
 *  Responsibilities:
 *    1. Generic TCP server helpers (accept, error, poll, sent, send, close)
 *       reusable by any server without modification.
 *    2. LwIP polling task (ethernetif_input + sys_check_timeouts).
 *    3. Top-level LI_Eth_Init() that starts all TCP servers and the task.
 *
 *  Adding a new TCP server requires only:
 *    - An init function that binds, listens and calls tcp_accept with a
 *      thin wrapper that delegates to li_tcp_server_accept().
 *    - A recv callback that implements the protocol-specific logic.
 */

#include "LI_Eth.h"
#include "tcp_modserver.h"
#include "ethernetif.h"
#include "lwip/timeouts.h"
#include "lwip/memp.h"
#include "cmsis_os.h"

/* --------------------------------------------------------------------------- */
/* Generic TCP server helpers                                                   */
/* --------------------------------------------------------------------------- */

err_t li_tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err,
                            tcp_recv_fn recv_cb)
{
    li_tcp_conn_t *conn;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);

    tcp_setprio(newpcb, TCP_PRIO_MIN);

    conn = (li_tcp_conn_t *)mem_malloc(sizeof(li_tcp_conn_t));
    if (conn != NULL)
    {
        conn->state   = LI_TCP_CONN_ACCEPTED;
        conn->pcb     = newpcb;
        conn->retries = 0;
        conn->p       = NULL;

        tcp_arg(newpcb, conn);
        tcp_recv(newpcb, recv_cb);
        tcp_err(newpcb, li_tcp_server_error);
        tcp_poll(newpcb, li_tcp_server_poll, 0);

        return ERR_OK;
    }

    /* Allocation failed – close before returning the error */
    li_tcp_server_connection_close(newpcb, NULL);
    return ERR_MEM;
}

void li_tcp_server_error(void *arg, err_t err)
{
    li_tcp_conn_t *conn = (li_tcp_conn_t *)arg;

    LWIP_UNUSED_ARG(err);

    if (conn != NULL)
    {
        mem_free(conn);
    }
}

err_t li_tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
    li_tcp_conn_t *conn = (li_tcp_conn_t *)arg;

    if (conn != NULL)
    {
        if (conn->p != NULL)
        {
            tcp_sent(tpcb, li_tcp_server_sent);
            li_tcp_server_send(tpcb, conn);
        }
        else if (conn->state == LI_TCP_CONN_CLOSING)
        {
            li_tcp_server_connection_close(tpcb, conn);
        }
        return ERR_OK;
    }

    tcp_abort(tpcb);
    return ERR_ABRT;
}

err_t li_tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    li_tcp_conn_t *conn = (li_tcp_conn_t *)arg;

    LWIP_UNUSED_ARG(len);

    conn->retries = 0;

    if (conn->p != NULL)
    {
        tcp_sent(tpcb, li_tcp_server_sent);
        li_tcp_server_send(tpcb, conn);
    }
    else if (conn->state == LI_TCP_CONN_CLOSING)
    {
        li_tcp_server_connection_close(tpcb, conn);
    }

    return ERR_OK;
}

void li_tcp_server_send(struct tcp_pcb *tpcb, li_tcp_conn_t *conn)
{
    struct pbuf *ptr;
    err_t        wr_err = ERR_OK;

    while ((wr_err == ERR_OK) &&
           (conn->p != NULL) &&
           (conn->p->len <= tcp_sndbuf(tpcb)))
    {
        ptr    = conn->p;
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);

        if (wr_err == ERR_OK)
        {
            u16_t plen;
            u8_t  freed;

            plen      = ptr->len;
            conn->p   = ptr->next;

            if (conn->p != NULL)
            {
                pbuf_ref(conn->p);
            }

            do { freed = pbuf_free(ptr); } while (freed == 0);

            tcp_recved(tpcb, plen);
        }
        else if (wr_err == ERR_MEM)
        {
            conn->p = ptr;
        }
    }
}

void li_tcp_server_connection_close(struct tcp_pcb *tpcb, li_tcp_conn_t *conn)
{
    tcp_arg(tpcb,  NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb,  NULL);
    tcp_poll(tpcb, NULL, 0);

    if (conn != NULL)
    {
        mem_free(conn);
    }

    tcp_close(tpcb);
}

/* --------------------------------------------------------------------------- */
/* Private polling task                                                         */
/* --------------------------------------------------------------------------- */

static void li_eth_polling_task(void *argument)
{
    extern struct netif gnetif;
    (void)argument;

    for (;;)
    {
        ethernetif_input(&gnetif);
        sys_check_timeouts();
    }
}

/* --------------------------------------------------------------------------- */
/* Public API implementation                                                    */
/* --------------------------------------------------------------------------- */

void LI_Eth_Init(void)
{
    /* Start all TCP servers */
    tcp_modserver_init();

    /* Create the Ethernet polling task */
    static const osThreadAttr_t eth_task_attr = {
        .name       = "ethPolling",
        .stack_size = 256 * 4,
        .priority   = (osPriority_t) osPriorityNormal,
    };
    osThreadNew(li_eth_polling_task, NULL, &eth_task_attr);
}
