/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is Vision. 
 * 
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 * 
 * Contributor(s): Rene Gollent
 */
 
#include <Button.h>
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Window.h>

#include "NumericFilter.h"
#include "ServerEntryWindow.h"
#include "Vision.h"
#include "VTextControl.h"

#include <stdio.h>
#include <stdlib.h>

const uint32 M_SERVER_NAME_CHANGED = 'sesn';
const uint32 M_SERVER_PORT_CHANGED = 'sesp';
const uint32 M_SERVER_DONE         = 'sesd';
const uint32 M_SERVER_CANCEL       = 'sesc';
const uint32 M_SERVER_STATE        = 'sess';


ServerEntryWindow::ServerEntryWindow (BHandler *handler, BMessage *invoked, const ServerData *data)
  : BWindow (
        BRect (50, 50, 350, 250), 
        "Add Server", 
        B_FLOATING_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS)
{
  AddChild (new ServerEntryView (handler, invoked, data));
}

ServerEntryWindow::~ServerEntryWindow (void)
{
}

ServerEntryView::ServerEntryView (BHandler *handler, BMessage *invoked, const ServerData *data)
  : BView (
      BRect (0, 0, 0, 0),
      "entryView",
      B_FOLLOW_ALL_SIDES,
      B_WILL_DRAW),
        invocation (invoked),
        target (handler),
        currentServer (data)
{
	ASSERT (handler != NULL);
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	serverName = new VTextControl (BRect (0,0,0,0), "serverName", "Server: ",
		(data) ? data->serverName : "", new BMessage (M_SERVER_NAME_CHANGED),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_NAVIGABLE_JUMP, true);
	BString strPort ("");
	if (data) strPort << data->port;
	else strPort << 6667;
	port = new VTextControl (BRect (0,0,0,0), "portVal", "Port: ",
		strPort.String(), new BMessage (M_SERVER_PORT_CHANGED),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_NAVIGABLE, true);
    port->SetDivider (be_plain_font->StringWidth ("Port: ") + 5);

    BMenu *stateMenu = new BMenu ("Choose status");
    stateMenu->AddItem (new BMenuItem ("Primary", new BMessage (M_SERVER_STATE)));
    stateMenu->AddItem (new BMenuItem ("Secondary", new BMessage (M_SERVER_STATE)));
    stateMenu->AddItem (new BMenuItem ("Disabled" , new BMessage (M_SERVER_STATE)));
    statusField = new BMenuField (BRect (0,0,0,0), "states", "State: ", stateMenu,
      B_FOLLOW_LEFT | B_FOLLOW_TOP, B_NAVIGABLE_JUMP | B_WILL_DRAW);

    okButton = new BButton (BRect (0,0,0,0), "serverOk", "Done",
      new BMessage (M_SERVER_DONE), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_NAVIGABLE | B_WILL_DRAW);
    
    cancelButton = new BButton (BRect (0,0,0,0), "serverCancel", "Cancel",
      new BMessage (M_SERVER_CANCEL), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_NAVIGABLE | B_WILL_DRAW);

    AddChild (serverName);
    AddChild (port);
	AddChild (statusField);
	AddChild (okButton);
	AddChild (cancelButton);
}


ServerEntryView::~ServerEntryView (void)
{
	delete invocation;
}

void
ServerEntryView::AttachedToWindow (void)
{
  BRect bounds (Window()->Bounds());
  ResizeTo (bounds.Width(), bounds.Height());
  serverName->ResizeToPreferred();
  serverName->SetDivider (be_plain_font->StringWidth ("Server: ") + 5);
  serverName->ResizeTo (Bounds().Width() / 2, serverName->Bounds().Height());
  serverName->MoveTo (10,10);
  serverName->SetTarget (this);
  
  port->ResizeToPreferred();
  port->MoveTo (serverName->Frame().right + 15, serverName->Frame().top);
  port->SetTarget (this);
  port->TextView()->AddFilter (new NumericFilter());
  
  float diff (0);
  if ((diff = (port->Frame().right - Bounds().right)) > 0.0)
     ResizeBy (diff, 0.0);
  
  statusField->ResizeToPreferred();     
  statusField->Menu()->SetTargetForItems (this);

#if B_BEOS_VERSION_DANO
  statusField->MoveTo ((Bounds().Width() - statusField->Bounds().Width()) / 2.0,
#else
  statusField->MoveTo ((Bounds().Width() - statusField->Bounds().Width()) / 4.0,
#endif
  	serverName->Frame().bottom + 15); 
  
  cancelButton->SetTarget (this);
  okButton->SetTarget (this);
  okButton->ResizeToPreferred();
  okButton->MoveTo (port->Frame().right - okButton->Bounds().Width(), statusField->Frame().bottom + 30);
  cancelButton->ResizeToPreferred();
  cancelButton->MoveTo (okButton->Frame().left - (cancelButton->Frame().Width() + 5), okButton->Frame().top);

  okButton->SetEnabled (false);
    
  Window()->ResizeTo (okButton->Frame().right + 5, okButton->Frame().bottom + 10);
  
  BView::AttachedToWindow();

  if (okButton->Frame().bottom > Bounds().bottom)
    ResizeTo (Bounds().Width(), okButton->Frame().bottom + 5);

  if (currentServer)
	dynamic_cast<BInvoker *>(statusField->Menu()->ItemAt(currentServer->state))->Invoke();
  BView::AttachedToWindow();
}

void
ServerEntryView::AllAttached (void)
{
  port->MakeFocus (false);
  port->MakeFocus (true);
  serverName->MakeFocus(false);
  serverName->MakeFocus(true);
  BView::AllAttached();
}

void
ServerEntryView::CheckDoneState (void)
{
	if (serverName->TextView()->TextLength() != 0)
		if (port->TextView()->TextLength() != 0)
			if (statusField->Menu()->FindMarked() != NULL)
			{
				okButton->SetEnabled (true);
				return;
			}
	okButton->SetEnabled (false);
}

void
ServerEntryView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_SERVER_STATE:
			{
				BMenu *menu (statusField->Menu());
				BMenuItem *item (NULL);
				item = menu->FindMarked();
				if (item)
					item->SetMarked (false);
				msg->FindPointer ("source", reinterpret_cast<void **>(&item));
				item->SetMarked (true);
				statusField->MenuItem()->SetLabel (item->Label());
			}

		case M_SERVER_NAME_CHANGED:
		case M_SERVER_PORT_CHANGED:
			CheckDoneState();
			break;
		
		case M_SERVER_DONE:
			{
				BMenu *menu (statusField->Menu());
				ServerData data;
				memset (data.serverName, 0, sizeof (data.serverName));
				strcpy (data.serverName, serverName->Text());
				data.port = atoi (port->Text());
				data.state = menu->IndexOf(menu->FindMarked());
				BMessenger msgr (target);
				BMessage msg (*invocation);
				msg.AddData ("server", B_RAW_TYPE, &data, sizeof(data));
				msgr.SendMessage (&msg);
			}
		
		case M_SERVER_CANCEL:
		 	Window()->PostMessage (B_QUIT_REQUESTED);
		 	break;
		
		default:
			BView::MessageReceived (msg);
	}
}