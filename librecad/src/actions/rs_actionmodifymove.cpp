/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#include "rs_actionmodifymove.h"

#include <QAction>
#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_coordinateevent.h"
#include "rs_modification.h"

RS_ActionModifyMove::RS_ActionModifyMove(RS_EntityContainer& container,
        RS_GraphicView& graphicView)
        :RS_PreviewActionInterface("Move Entities",
						   container, graphicView)
		,data(new RS_MoveData())
{
	actionType=RS2::ActionModifyMove;
}


QAction* RS_ActionModifyMove::createGUIAction(RS2::ActionType /*type*/, QObject* /*parent*/) {
	QAction* action = new QAction(QIcon(":/extui/modifymove.png"), tr("&Move / Copy"),  NULL);
    return action;
}

RS_ActionModifyMove::~RS_ActionModifyMove(){}

void RS_ActionModifyMove::trigger() {

    RS_DEBUG->print("RS_ActionModifyMove::trigger()");

    RS_Modification m(*container, graphicView);
	m.move(*data);

    RS_DIALOGFACTORY->updateSelectionWidget(container->countSelected(),container->totalSelectedLength());
    finish(false);
}



void RS_ActionModifyMove::mouseMoveEvent(QMouseEvent* e) {
    RS_DEBUG->print("RS_ActionModifyMove::mouseMoveEvent begin");

    if (getStatus()==SetReferencePoint ||
            getStatus()==SetTargetPoint) {

        RS_Vector mouse = snapPoint(e);
        switch (getStatus()) {
        case SetReferencePoint:
            referencePoint = mouse;
            break;

        case SetTargetPoint:
            if (referencePoint.valid) {
                targetPoint = mouse;

                deletePreview();
                preview->addSelectionFrom(*container);
                preview->move(targetPoint-referencePoint);
                drawPreview();
            }
            break;

        default:
            break;
        }
    }

    RS_DEBUG->print("RS_ActionModifyMove::mouseMoveEvent end");
}



void RS_ActionModifyMove::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button()==Qt::LeftButton) {
        RS_CoordinateEvent ce(snapPoint(e));
        coordinateEvent(&ce);
    } else if (e->button()==Qt::RightButton) {
        deletePreview();
        init(getStatus()-1);
    }
}

void RS_ActionModifyMove::coordinateEvent(RS_CoordinateEvent* e) {

    if (e==NULL) {
        return;
    }

    RS_Vector pos = e->getCoordinate();

    switch (getStatus()) {
    case SetReferencePoint:
        referencePoint = pos;
        graphicView->moveRelativeZero(referencePoint);
        setStatus(SetTargetPoint);
        break;

    case SetTargetPoint:
        targetPoint = pos;
        graphicView->moveRelativeZero(targetPoint);
        setStatus(ShowDialog);
		if (RS_DIALOGFACTORY->requestMoveDialog(*data)) {
			if(data->number<0){
				data->number=abs(data->number);
				RS_DIALOGFACTORY->commandMessage(QString(tr("Invalid number of copies, use %1 ")).arg(data->number));
            }
			data->offset = targetPoint - referencePoint;
            trigger();
        }
        break;

    default:
        break;
    }
}


void RS_ActionModifyMove::updateMouseButtonHints() {
    if(RS_DIALOGFACTORY != NULL) {
        switch (getStatus()) {
        /*case Select:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Pick entities to move"),
                                           tr("Cancel"));
            break;*/
        case SetReferencePoint:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify reference point"),
                                                tr("Cancel"));
            break;
        case SetTargetPoint:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify target point"),
                                                tr("Back"));
            break;
        default:
            RS_DIALOGFACTORY->updateMouseWidget();
            break;
        }
    }
}



void RS_ActionModifyMove::updateMouseCursor() {
        if(graphicView != NULL){
    graphicView->setMouseCursor(RS2::CadCursor);
        }
}

// EOF
