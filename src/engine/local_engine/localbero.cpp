/***************************************************************************
 *   Copyright (C) 2006 by FThauer FHammer   *
 *   f.thauer@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "localbero.h"

#include "localexception.h"
#include "engine_msg.h"
#include <core/loghelper.h>

using namespace std;

LocalBeRo::LocalBeRo(HandInterface* hi, int id, unsigned dP, int sB, GameState gS)
: BeRoInterface(), myHand(hi), myBeRoID(gS), myID(id), dealerPosition(dP), smallBlindPosition(0), dealerPositionId(dP), smallBlindPositionId(0), bigBlindPositionId(0), smallBlind(sB), highestSet(0), minimumRaise(2*sB), fullBetRule(false), firstRun(true), firstRunGui(true), firstRound(true), firstHeadsUpRound(true), currentPlayersTurnId(0), firstRoundLastPlayersTurnId(0), logBoardCardsDone(false)
{
	currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();
	lastPlayersTurnIt = myHand->getRunningPlayerList()->begin();

	PlayerListConstIterator it_c;

	// determine bigBlindPosition
	for(it_c=myHand->getActivePlayerList()->begin(); it_c!=myHand->getActivePlayerList()->end(); it_c++) {
		if((*it_c)->getMyButton() == BUTTON_BIG_BLIND) {
			bigBlindPositionId = (*it_c)->getMyUniqueID();
			break;
		}
	}
	if(it_c == myHand->getActivePlayerList()->end()) {
		throw LocalException(__FILE__, __LINE__, ERR_ACTIVE_PLAYER_NOT_FOUND);
	}

	// determine smallBlindPosition
	for(it_c=myHand->getActivePlayerList()->begin(); it_c!=myHand->getActivePlayerList()->end(); it_c++) {
		if((*it_c)->getMyButton() == BUTTON_SMALL_BLIND) {
			smallBlindPositionId = (*it_c)->getMyUniqueID();
			break;
		}
	}
	if(it_c == myHand->getActivePlayerList()->end()) {
		throw LocalException(__FILE__, __LINE__, ERR_ACTIVE_PLAYER_NOT_FOUND);
	}

}


LocalBeRo::~LocalBeRo()
{
}

int LocalBeRo::getHighestCardsValue() const
{
	LOG_ERROR(__FILE__ << " (" << __LINE__ << "): getHighestCardsValue() in wrong BeRo");
	return 0;
}

void LocalBeRo::nextPlayer() {

	PlayerListConstIterator currentPlayersTurnConstIt = myHand->getRunningPlayerIt(currentPlayersTurnId);
	if(currentPlayersTurnConstIt == myHand->getRunningPlayerList()->end()) {
		throw LocalException(__FILE__, __LINE__, ERR_RUNNING_PLAYER_NOT_FOUND);
	}

	(*currentPlayersTurnConstIt)->action();

}

void LocalBeRo::run() {

	if(firstRunGui) {
		firstRunGui = false;
		myHand->setLastPlayersTurn(-1);
		myHand->getGuiInterface()->dealBeRoCards(myBeRoID);
	}
	else {

		if(firstRun) {

			firstRun = false;
	
			if(!(myHand->getAllInCondition())) {

				PlayerListIterator it_1, it_2;
				size_t i;
			
				// running player before smallBlind
				bool formerRunningPlayerFound = false;
				if(myHand->getActivePlayerList()->size() > 2) {
		
					it_1 = myHand->getActivePlayerIt(smallBlindPositionId);
					if(it_1 == myHand->getActivePlayerList()->end()) {
						throw LocalException(__FILE__, __LINE__, ERR_ACTIVE_PLAYER_NOT_FOUND);
					}
		
					for(i=0; i<myHand->getActivePlayerList()->size(); i++) {	
		
						if(it_1 == myHand->getActivePlayerList()->begin()) it_1 = myHand->getActivePlayerList()->end();
						it_1--;
		
						it_2 = myHand->getRunningPlayerIt((*it_1)->getMyUniqueID());
						// running player found
						if(it_2 != myHand->getRunningPlayerList()->end()) {
							firstRoundLastPlayersTurnId = (*it_2)->getMyUniqueID();
							formerRunningPlayerFound = true;
							break;
						}
					}
					if(!formerRunningPlayerFound) {
						throw LocalException(__FILE__, __LINE__, ERR_FORMER_RUNNING_PLAYER_NOT_FOUND);
					}
				}
				// heads up: bigBlind begins -> dealer/smallBlind is running player before bigBlind
				else {
					firstRoundLastPlayersTurnId = smallBlindPositionId;
				}	
				currentPlayersTurnId = firstRoundLastPlayersTurnId;
			}
		}

		//log the turned cards
		if(!logBoardCardsDone) {

			int tempBoardCardsArray[5];

			myHand->getBoard()->getMyCards(tempBoardCardsArray);

			switch(myBeRoID) {
				case GAME_STATE_FLOP: myHand->getGuiInterface()->logDealBoardCardsMsg(myBeRoID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2]);
				break;
				case GAME_STATE_TURN: myHand->getGuiInterface()->logDealBoardCardsMsg(myBeRoID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2], tempBoardCardsArray[3]);
				break;
				case GAME_STATE_RIVER: myHand->getGuiInterface()->logDealBoardCardsMsg(myBeRoID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2], tempBoardCardsArray[3], tempBoardCardsArray[4]);
				break;
				default: { LOG_ERROR(__FILE__ << " (" << __LINE__ << "): ERROR - wrong myBeRoID"); }
			}
			logBoardCardsDone = true;

		}

		bool allHighestSet = true;

		PlayerListIterator it;
		PlayerListIterator it_c;


		// check if all running players have same sets (else allHighestSet = false)
		for( it_c = myHand->getRunningPlayerList()->begin(); it_c != myHand->getRunningPlayerList()->end(); it_c++) {
			if(highestSet != (*it_c)->getMySet()) {
				allHighestSet = false;
				break;
			}
		}

		int i;

		// prfen, ob aktuelle bero wirklich dran ist
		if(!firstRound && allHighestSet) { 
	
			// aktuelle bero nicht dran, weil alle Sets gleich sind
			//also gehe in naechste bero
			myHand->setCurrentRound(myBeRoID+1);

			//Action loeschen und ActionButtons refresh
			for(it_c=myHand->getRunningPlayerList()->begin(); it_c!=myHand->getRunningPlayerList()->end(); it_c++) {
				(*it_c)->setMyAction(PLAYER_ACTION_NONE);
			}

			//Sets in den Pot verschieben und Sets = 0 und Pot-refresh
			myHand->getBoard()->collectSets();
			myHand->getBoard()->collectPot();
			myHand->getGuiInterface()->refreshPot();
			
			myHand->getGuiInterface()->refreshSet();
			myHand->getGuiInterface()->refreshCash();
			for(i=0; i<MAX_NUMBER_OF_PLAYERS; i++) { myHand->getGuiInterface()->refreshAction(i,PLAYER_ACTION_NONE); }
			
			myHand->switchRounds();
		}
		else {
			// aktuelle bero ist wirklich dran

			// Anzahl der effektiv gespielten Runden (des human player) erhöhen
			it_c = myHand->getActivePlayerIt(0);
			if( it_c != myHand->getActivePlayerList()->end() ) {
				// human player is active
				if( (*it_c)->getMyAction() != PLAYER_ACTION_FOLD ) {
					myHand->setBettingRoundsPlayed(myBeRoID);
				}
			}

			// determine next running player
			PlayerListConstIterator currentPlayersTurnIt = myHand->getRunningPlayerIt( currentPlayersTurnId );
			if(currentPlayersTurnIt == myHand->getRunningPlayerList()->end()) {
				throw LocalException(__FILE__, __LINE__, ERR_RUNNING_PLAYER_NOT_FOUND);
			}
			
			currentPlayersTurnIt++;
			if(currentPlayersTurnIt == myHand->getRunningPlayerList()->end()) currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();
			
			currentPlayersTurnId = (*currentPlayersTurnIt)->getMyUniqueID();

			//highlight active players groupbox and clear action
			myHand->getGuiInterface()->refreshGroupbox(currentPlayersTurnId,2);
			myHand->getGuiInterface()->refreshAction(currentPlayersTurnId,0);

			currentPlayersTurnIt = myHand->getRunningPlayerIt( currentPlayersTurnId );
			if(currentPlayersTurnIt == myHand->getRunningPlayerList()->end()) {
				throw LocalException(__FILE__, __LINE__, ERR_RUNNING_PLAYER_NOT_FOUND);
			}

			(*currentPlayersTurnIt)->setMyTurn(true);


			if( currentPlayersTurnId == firstRoundLastPlayersTurnId ) {
				firstRound = false;
			}

			if( currentPlayersTurnId == 0) {
				// Wir sind dran
				myHand->getGuiInterface()->meInAction();
			}
			else {

				//Gegner sind dran
				myHand->getGuiInterface()->beRoAnimation2(myBeRoID);
			}
		}
	}
}
