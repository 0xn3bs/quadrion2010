/*
 *  qEventLib
 *  qEvent.cpp
 *
 *	Copyright (c) 2001, AVS
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions are met:
 *	1.	Redistributions of source code must retain the above copyright
 *		notice, this list of conditions and the following disclaimer.
 *	2.	Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in the
 *		documentation and/or other materials provided with the distribution.
 *	3.	All advertising materials mentioning features or use of this software
 *		must display the following acknowledgement:
 *		This product includes software developed by the AVS.
 *	4.	Neither the name of the AVS nor the
 *		names of its contributors may be used to endorse or promote products
 *		derived from this software without specific prior written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY AVS ''AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *	DISCLAIMED. IN NO EVENT SHALL AVS BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include "qscriptengine.h"
#include "qevent.h"


		qevent::qevent()
		{
		}

		qevent::qevent(unsigned int _param1, unsigned int _param2, event_type _type)
		{
			this->data.type = _type;
			this->data.key = EVENT_DEFAULT;
			
			switch (_type)
			{
				case EVENT_KEY:
				{
					this->data.evt_data.key_d.key = _param1;
					this->data.evt_data.key_d.state = _param2;
					break;
				}
				case EVENT_MOUSE:
				{
					this->data.evt_data.mouse_d.x = _param1;
					this->data.evt_data.mouse_d.y = _param2;
					break;
				}
				default:
					break;
			}
		}

		event_type qevent::type()
		{
			return this->data.type;
		}

		unsigned int qevent::key()
		{
			return this->data.key;
		}
		
		unsigned int qevent::get_key_code()
		{
			return this->data.evt_data.key_d.key;
		}
		
		unsigned int qevent::get_key_state()
		{
			return this->data.evt_data.key_d.state;
		}
		
		unsigned int qevent::get_mouse_x()
		{
			return this->data.evt_data.mouse_d.x;
		}
		
		unsigned int qevent::get_mouse_y()
		{
			return this->data.evt_data.mouse_d.y;
		}
		
		unsigned int qevent::get_mouse_dx()
		{
			return this->data.evt_data.mouse_d.dx;
		}
		
		unsigned int qevent::get_mouse_dy()
		{
			return this->data.evt_data.mouse_d.dy;
		}
		
		
		
		void qevent::REGISTER_SCRIPTABLES(qscriptengine *engine)
		{
			REGISTER_CLASS(engine, "qevent", qevent);
			
			REGISTER_METHOD(engine, "qevent", qevent, "uint type()", type);
			REGISTER_METHOD(engine, "qevent", qevent, "uint key()", key);
			REGISTER_METHOD(engine, "qevent", qevent, "uint get_key_code()", get_key_code);
			REGISTER_METHOD(engine, "qevent", qevent, "uint get_key_state()", get_key_state);
			REGISTER_METHOD(engine, "qevent", qevent, "uint get_mouse_x()", get_mouse_x);
			REGISTER_METHOD(engine, "qevent", qevent, "uint get_mouse_y()", get_mouse_y);
			REGISTER_METHOD(engine, "qevent", qevent, "uint get_mouse_dx()", get_mouse_dx);
			REGISTER_METHOD(engine, "qevent", qevent, "uint get_mouse_dy()", get_mouse_dy);
			
			engine->getEngine()->RegisterEnum("event_type");
			engine->getEngine()->RegisterEnumValue("event_type", "EVENT_DEFAULT", 0);
			engine->getEngine()->RegisterEnumValue("event_type", "EVENT_KEY", 1);
			engine->getEngine()->RegisterEnumValue("event_type", "EVENT_MOUSE", 2);
			engine->getEngine()->RegisterEnumValue("event_type", "EVENT_GAME", 3);
			engine->getEngine()->RegisterEnumValue("event_type", "EVENT_USER", 4);
			engine->getEngine()->RegisterEnumValue("event_type", "EVENT_MONITOR", 5);
			
			engine->getEngine()->RegisterEnum("key_state");
			engine->getEngine()->RegisterEnumValue("key_state", "KEY_UP", 0);
			engine->getEngine()->RegisterEnumValue("key_state", "KEY_PRESSED", 1);
			engine->getEngine()->RegisterEnumValue("key_state", "KEY_DOWN", 2);
			engine->getEngine()->RegisterEnumValue("key_state", "KEY_RELEASED", 3);

			engine->getEngine()->RegisterEnum("VK_KEYS");
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_0", 48);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_1", 0x31);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_2", 0x32);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_3", 0x33);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_4", 0x34);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_5", 0x35);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_6", 0x36);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_7", 0x37);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_8", 0x38);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_9", 0x39);

			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_A", 0x41);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_B", 0x42);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_C", 0x43);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_D", 0x44);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_E", 0x45);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_F", 0x46);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_G", 0x47);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_H", 0x48);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_I", 0x49);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_J", 0x4A);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_K", 0x4B);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_L", 0x4C);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_M", 0x4D);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_N", 0x4E);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_O", 0x4F);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_P", 0x50);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_Q", 0x51);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_R", 0x52);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_S", 0x53);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_T", 0x54);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_U", 0x55);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_V", 0x56);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_W", 0x57);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_X", 0x58);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_Y", 0x59);
			engine->getEngine()->RegisterEnumValue("VK_KEYS", "VK_Z", 0x5A);
		}