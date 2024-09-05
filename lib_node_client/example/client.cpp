/*
 * Copyright (c) 2023
 * Yann Charbon <yann.charbon@heig-vd.h>
 *
 */

 /*******************************************************************************
  *
  * Copyright (c) 2013, 2014, 2015 Intel Corporation and others.
  * All rights reserved. This program and the accompanying materials
  * are made available under the terms of the Eclipse Public License v2.0
  * and Eclipse Distribution License v1.0 which accompany this distribution.
  *
  * The Eclipse Public License is available at
  *    http://www.eclipse.org/legal/epl-v20.html
  * The Eclipse Distribution License is available at
  *    http://www.eclipse.org/org/documents/edl-v10.php.
  *
  * Contributors:
  *    David Navarro, Intel Corporation - initial API and implementation
  *    Benjamin Cabé - Please refer to git log
  *    Fabien Fleutot - Please refer to git log
  *    Simon Bernard - Please refer to git log
  *    Julien Vermillard - Please refer to git log
  *    Axel Lorente - Please refer to git log
  *    Toby Jaffey - Please refer to git log
  *    Bosch Software Innovations GmbH - Please refer to git log
  *    Pascal Rieux - Please refer to git log
  *    Christian Renz - Please refer to git log
  *
  *******************************************************************************/

  /*
   Copyright (c) 2013, 2014 Intel Corporation

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

       * Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright notice,
         this list of conditions and the following disclaimer in the documentation
         and/or other materials provided with the distribution.
       * Neither the name of Intel Corporation nor the names of its contributors
         may be used to endorse or promote products derived from this software
         without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
   THE POSSIBILITY OF SUCH DAMAGE.

   David Navarro <david.navarro@intel.com>
   Bosch Software Innovations GmbH - Please refer to git log

  */

  /**
   *  Copyright (c) 2024
   * 
   *  @file client.cpp
   *  @brief This source file is an example of main program using the NodeClient from lib_node_client library.
   *
   *  @author Bastien Pillonel <bastien.pillonel@heig-vd.ch>
   *
   */

#if !MBED_TEST_MODE

#include "objects_definition.h"

   // Connect to the Leshan test server as default: http://leshan.eclipse.org
#define M2M_SERVER_URL "2a01:111:f100:9001::1761:93fa" // LESHAN
#define CLIENT_ENDPOINT_NAME "mbedM2M"
#define CLIENT_IDENTITY "mbedM2M"
#define CLIENT_KEYSTR "65875A0C3D4646A99BFC4D5967EE7DB3"
#define SERVER_DTLS_PORT "5684"
#define SERVER_PORT "5683"

int main(int argc, char *argv[])
{
    ThisThread::sleep_for(1s);

    std::vector<NodeObject *> *objects = initializeObjects();

    NodeClient client = { objects, nullptr, M2M_SERVER_URL, SERVER_DTLS_PORT, CLIENT_KEYSTR, CLIENT_ENDPOINT_NAME, CLIENT_IDENTITY };

    client.InitNetwork();
    client.StartClient();
    return 0;
}

#endif
