/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable echo application.
 *
 **/

 /* This file was modified by ST */

/*
 * tcp_modserver.c — Modbus TCP server
 *
 * Contains only the Modbus-specific logic:
 *   - tcp_modserver_init()   : binds to LI_MODBUS_TCP_PORT and starts listening.
 *   - tcp_modserver_accept() : thin LwIP callback that delegates connection
 *                              management to LI_Eth generic helpers.
 *   - tcp_modserver_recv()   : parses Modbus TCP frames via LI_Modbus_TCP_Process().
 *
 * All generic TCP connection management (error, poll, sent, send, close) lives
 * in LI_Eth.c and is accessed through LI_Eth.h.
 */

#include "tcp_modserver.h"
#include "LI_Eth.h"
#include "LI_modbus.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"

#if LWIP_TCP

static struct tcp_pcb *tcp_modserver_pcb;

static err_t tcp_modserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_modserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

/**
  * @brief  Initialises the Modbus TCP server.
  *
  * Binds to LI_MODBUS_TCP_PORT on all interfaces and starts listening.
  * Connection management is handled by the generic LI_Eth helpers.
  */
void tcp_modserver_init(void)
{
    tcp_modserver_pcb = tcp_new();

    if (tcp_modserver_pcb != NULL)
    {
        err_t err = tcp_bind(tcp_modserver_pcb, IP_ADDR_ANY, LI_MODBUS_TCP_PORT);

        if (err == ERR_OK)
        {
            tcp_modserver_pcb = tcp_listen(tcp_modserver_pcb);
            tcp_accept(tcp_modserver_pcb, tcp_modserver_accept);
        }
        else
        {
            memp_free(MEMP_TCP_PCB, tcp_modserver_pcb);
        }
    }
}

/**
  * @brief  LwIP tcp_accept callback.
  *
  * Delegates connection setup to li_tcp_server_accept(), supplying the
  * Modbus-specific recv callback.
  */
static err_t tcp_modserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    return li_tcp_server_accept(arg, newpcb, err, tcp_modserver_recv);
}

/**
  * @brief  LwIP tcp_recv callback — Modbus TCP frame processing.
  *
  * Calls LI_Modbus_TCP_Process() on every received frame and writes the
  * response back on the same connection.
  */
static err_t tcp_modserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    li_tcp_conn_t *conn = (li_tcp_conn_t *)arg;
    err_t          ret_err;

    LWIP_ASSERT("arg != NULL", arg != NULL);

    if (p == NULL)
    {
        /* Remote host closed the connection */
        conn->state = LI_TCP_CONN_CLOSING;
        li_tcp_server_connection_close(tpcb, conn);
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        conn->p = NULL;
        pbuf_free(p);
        return err;
    }

    if (conn->state == LI_TCP_CONN_ACCEPTED || conn->state == LI_TCP_CONN_RECEIVED)
    {
        uint8_t  modbus_resp[MODBUS_TCP_MAX_ADU_SIZE];
        uint16_t modbus_resp_len = 0U;

        conn->state = LI_TCP_CONN_RECEIVED;
        tcp_recved(tpcb, p->tot_len);

        if (p->next != NULL)
        {
            /* Flatten fragmented pbuf chain into a contiguous buffer */
            uint8_t  flat_buf[MODBUS_TCP_MAX_ADU_SIZE];
            uint16_t flat_len = p->tot_len;
            pbuf_copy_partial(p, flat_buf, flat_len, 0);
            pbuf_free(p);

            if (LI_Modbus_TCP_Process(flat_buf, flat_len,
                                      modbus_resp, &modbus_resp_len) == MODBUS_OK)
            {
                tcp_write(tpcb, modbus_resp, modbus_resp_len, TCP_WRITE_FLAG_COPY);
                tcp_output(tpcb);
            }
        }
        else
        {
            if (LI_Modbus_TCP_Process((const uint8_t *)p->payload, p->len,
                                      modbus_resp, &modbus_resp_len) == MODBUS_OK)
            {
                tcp_write(tpcb, modbus_resp, modbus_resp_len, TCP_WRITE_FLAG_COPY);
                tcp_output(tpcb);
            }
            pbuf_free(p);
        }

        conn->p = NULL;
        ret_err = ERR_OK;
    }
    else
    {
        /* Unexpected state — discard data */
        tcp_recved(tpcb, p->tot_len);
        conn->p = NULL;
        pbuf_free(p);
        ret_err = ERR_OK;
    }

    return ret_err;
}

#endif /* LWIP_TCP */
