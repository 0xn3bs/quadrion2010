float moveSpeed = 1.0f;

class sMouse
{
	sMouse()
	{
	}
	
	void update(int _x, int _y)
	{
		this.dx = this.x - _x;
		this.dy = this.y - _y;
		this.x = _x;
		this.y = _y;
	}
	
	bool moved()
	{
		if(this.dx + this.dy > 0)
			return true;
		return false;
	}
	
	int x;
	int y;
	int dx;
	int dy;
}

class sKeyboard
{
	sKeyboard()
	{
		this.keys = uint8[](256, 0);
		this.oldKeys = uint8[](256, 0);
	}
	
	void update(uint8[]@ _keys)
	{
		for(uint8 a = 0;a < 256;a++)
		{
			this.oldKeys[a] = this.keys[a];
			this.keys[a] = _keys[a];
		}
	}
	
	void update()
	{
		for(int a = 0;a < 256;a++)
		{
			this.oldKeys[a] = this.keys[a];
			this.keys[a] = isKeyDown(a) ? 1 : 0;
		}
	}
	
	bool keyPressed(uint8 key)
	{
		return (this.keys[key] == 1 && this.oldKeys[key] == 0);
	}
	
	bool keyDown(uint8 key)
	{
		return (this.keys[key] == 1 && this.oldKeys[key] == 1);
	}
	
	bool keyReleased(uint8 key)
	{
		return (this.keys[key] == 0 && this.oldKeys[key] == 1);
	}
	
	bool keyUp(uint8 key)
	{
		return (this.keys[key] == 0 && this.oldKeys[key] == 0);
	}
		
	uint8[] keys;
	uint8[] oldKeys;
}

sKeyboard KB;
sMouse MOUSE;
CTimer timer;

void updateKeyboard()
{
	timer.Start();
	KB.update();
	int x;
	int y;
	GetMousePosition(x, y);
	MOUSE.update(x, y);
}

void updateMovement(CCamera @cam)
{	
	double dt = timer.GetElapsedSec();
	if(KB.keyDown(VK_A))
		cam.MoveCameraRelative(-moveSpeed*dt, 0.0f, 0.0f);
		
	if(KB.keyDown(VK_D))
		cam.MoveCameraRelative(moveSpeed*dt, 0.0f, 0.0f);
		
	if(KB.keyDown(VK_S))
		cam.MoveCameraRelative(0.0f, 0.0f, -moveSpeed*dt);
		
	if(KB.keyDown(VK_W))
		cam.MoveCameraRelative(0.0f, 0.0f, moveSpeed*dt);
	
	//if(MOUSE.moved())
	//	cam.RotateByMouse(MOUSE.x, MOUSE.y, 1400/2, 900/2);
	
	cam.Apply();
}

void ON_EVENT(qevent @evt, qobject @obj)
{
	CCamera @view = cast<CCamera>(@obj);
	switch(evt.type())
	{
		case EVENT_KEY :
		{
			switch(evt.get_key_code())
			{
				case VK_A :
				{
					view.MoveCameraRelative(-1.0f, 0.0f, 0.0f);
					break;
				}
				case VK_D :
				{
					view.MoveCameraRelative(1.0f, 0.0f, 0.0f);
					break;
				}
				case VK_W :
				{
					view.MoveCameraRelative(0.0f, 0.0f, 1.0f);
					break;
				}
				case VK_S :
				{
					view.MoveCameraRelative(0.0f, 0.0f, -1.0f);
					break;
				}
				case VK_Q :
				{
					killApp();
					break;
				}
			}
			break;
		}
		case EVENT_MOUSE :
		{
			//view.RotateByMouse( evt.get_mouse_x(), evt.get_mouse_y(), 1400 / 2, 900 / 2 );
			break;
		}
	}
	view.Apply();
}
