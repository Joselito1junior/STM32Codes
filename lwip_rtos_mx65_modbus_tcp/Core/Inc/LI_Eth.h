/*
 * LI_Eth.h
 *
 *  Created on: Apr 8, 2026
 *      Author: Joselito Junior
 *
 *  Ethernet transport layer.
 *
 *  Owns:
 *    1. The LwIP polling task (ethernetif_input + sys_check_timeouts).
 *    2. A set of generic TCP server helpers reusable by any server
 *       (tcp_modserver, http_server, …).
 *
 *  Each server provides only two things:
 *    - An init function that calls tcp_bind/tcp_listen/tcp_accept.
 *    - A recv callback with the protocol-specific frame handling.
 *  Everything else (accept, poll, sent, send, close, error) is handled
 *  here via li_tcp_server_accept() and the helper functions below.
 */

#ifndef INC_LI_ETH_H_
#define INC_LI_ETH_H_

#include "lwip/tcp.h"
#include "lwip/err.h"

/* --------------------------------------------------------------------------- */
/* Generic TCP connection types                                                 */
/* --------------------------------------------------------------------------- */

/** @brief Lifecycle states of a managed TCP connection. */
typedef enum
{
    LI_TCP_CONN_NONE = 0,  /**< No connection                   */
    LI_TCP_CONN_ACCEPTED,  /**< Connection accepted, idle        */
    LI_TCP_CONN_RECEIVED,  /**< Data received, processing        */
    LI_TCP_CONN_CLOSING,   /**< Tear-down in progress            */
} li_tcp_conn_state_t;

/** @brief Per-connection state maintained by the generic helpers. */
typedef struct
{
    li_tcp_conn_state_t  state;    /**< Current connection state          */
    u8_t                 retries;  /**< Send retry counter                */
    struct tcp_pcb      *pcb;      /**< LwIP PCB for this connection      */
    struct pbuf         *p;        /**< Pending outgoing pbuf chain       */
} li_tcp_conn_t;

/* --------------------------------------------------------------------------- */
/* Generic TCP server helpers                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Generic tcp_accept handler — allocates li_tcp_conn_t and registers
 *        the provided protocol-specific recv callback.
 *
 * Call this from every server's own (LwIP-compatible) accept callback:
 *
 *     static err_t my_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
 *     {
 *         return li_tcp_server_accept(arg, newpcb, err, my_server_recv);
 *     }
 *
 * @param arg      Passed through from LwIP (unused).
 * @param newpcb   Newly accepted PCB.
 * @param err      Error from LwIP (unused).
 * @param recv_cb  Protocol-specific receive callback to register for newpcb.
 * @return ERR_OK on success, ERR_MEM if allocation failed.
 */
err_t li_tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err,
                            tcp_recv_fn recv_cb);

/** @brief Generic tcp_err callback — frees the li_tcp_conn_t on fatal error. */
void  li_tcp_server_error(void *arg, err_t err);

/** @brief Generic tcp_poll callback — retries pending sends or closes. */
err_t li_tcp_server_poll(void *arg, struct tcp_pcb *tpcb);

/** @brief Generic tcp_sent callback — drains pending pbufs or closes. */
err_t li_tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

/** @brief Generic send helper — feeds pbuf chain into tcp_write. */
void  li_tcp_server_send(struct tcp_pcb *tpcb, li_tcp_conn_t *conn);

/** @brief Removes all LwIP callbacks, frees conn and closes the PCB. */
void  li_tcp_server_connection_close(struct tcp_pcb *tpcb, li_tcp_conn_t *conn);

/* --------------------------------------------------------------------------- */
/* Top-level init                                                               */
/* --------------------------------------------------------------------------- */

/**
 * @brief Initialise the Ethernet transport layer.
 *
 * - Starts all TCP servers (currently tcp_modserver).
 * - Creates the FreeRTOS task that drives the LwIP polling loop.
 *
 * Must be called once after MX_LWIP_Init() completes.
 */
void LI_Eth_Init(void);

#endif /* INC_LI_ETH_H_ */
