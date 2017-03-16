#include "parameters_controller.h"
#include "../constant.h"
#include "app.h"
#include <assert.h>
#include <string.h>

namespace Probability {

ParametersController::ContentView::ContentView(Responder * parentResponder, SelectableTableView * selectableTableView) :
  m_numberOfParameters(1),
  m_titleView(PointerTextView(KDText::FontSize::Small, "Choisir les parametres", 0.5f, 0.5f, Palette::GreyDark, Palette::WallScreen)),
  m_firstParameterDefinition(PointerTextView(KDText::FontSize::Small, nullptr, 0.5f, 0.5f, KDColorBlack, Palette::WallScreen)),
  m_secondParameterDefinition(PointerTextView(KDText::FontSize::Small, nullptr, 0.5f, 0.5f, KDColorBlack, Palette::WallScreen)),
  m_selectableTableView(selectableTableView)
{
}

void ParametersController::ContentView::drawRect(KDContext * ctx, KDRect rect) const {
  int tableHeight = m_selectableTableView->size().height()+ Metric::CommonTopMargin + Metric::CommonBottomMargin;
  ctx->fillRect(KDRect(0, tableHeight, bounds().width(), bounds().height() - tableHeight), Palette::WallScreen);
}

PointerTextView * ParametersController::ContentView::parameterDefinitionAtIndex(int index) {
  assert(index >= 0 && index < 2);
  if (index == 0) {
    return &m_firstParameterDefinition;
  }
  return &m_secondParameterDefinition;
}

void ParametersController::ContentView::setNumberOfParameters(int numberOfParameters) {
  m_numberOfParameters = numberOfParameters;
}

int ParametersController::ContentView::numberOfSubviews() const {
  return m_numberOfParameters+2;
}

View * ParametersController::ContentView::subviewAtIndex(int index) {
  assert(index >= 0 && index < 5);
  if (index == 0) {
    return &m_titleView;
  }
  if (index == 1) {
    return m_selectableTableView;
  }
  if (index == 2) {
    return &m_firstParameterDefinition;
  }
  return &m_secondParameterDefinition;
}

void ParametersController::ContentView::layoutSubviews() {
  KDCoordinate titleHeight = KDText::stringSize("", KDText::FontSize::Small).height()+k_titleMargin;
  m_titleView.setFrame(KDRect(0, 0, bounds().width(), titleHeight));
  KDCoordinate tableHeight = m_selectableTableView->size().height() + Metric::CommonTopMargin + Metric::CommonBottomMargin;
  m_selectableTableView->setFrame(KDRect(0, titleHeight, bounds().width(),  tableHeight));
  KDCoordinate textHeight = KDText::stringSize("", KDText::FontSize::Small).height();
  KDCoordinate defOrigin = (titleHeight+tableHeight)/2+(bounds().height()-textHeight)/2;
  m_secondParameterDefinition.setFrame(KDRectZero);
  if (m_numberOfParameters == 2) {
    defOrigin = (titleHeight+tableHeight)/2+(bounds().height()-2*textHeight-k_textMargin)/2;
    m_secondParameterDefinition.setFrame(KDRect(0, defOrigin+textHeight+k_textMargin, bounds().width(), textHeight));
  }
  m_firstParameterDefinition.setFrame(KDRect(0, defOrigin, bounds().width(), textHeight));
}

/* Parameters Controller */

ParametersController::ParametersController(Responder * parentResponder) :
  FloatParameterController(parentResponder, "Suivant"),
  m_menuListCell{PointerTableCellWithEditableText(&m_selectableTableView, this, m_draftTextBuffer),
    PointerTableCellWithEditableText(&m_selectableTableView, this, m_draftTextBuffer)},
  m_contentView(ContentView(this, &m_selectableTableView)),
  m_law(nullptr),
  m_calculationController(CalculationController(nullptr))
{
}

View * ParametersController::view() {
  return &m_contentView;
}

const char * ParametersController::title() const {
  if (m_law != nullptr) {
    return m_law->title();
  }
  return "";
}

void ParametersController::setLaw(Law * law) {
  m_law = law;
  if (m_law != nullptr) {
    m_contentView.setNumberOfParameters(m_law->numberOfParameter());
  }
  m_calculationController.setLaw(law);
}

void ParametersController::viewWillAppear() {
  for (int i = 0; i < m_law->numberOfParameter(); i++) {
    m_previousParameters[i] = parameterAtIndex(i);
    m_contentView.parameterDefinitionAtIndex(i)->setText(m_law->parameterDefinitionAtIndex(i));
  }
  m_contentView.layoutSubviews();
  FloatParameterController::viewWillAppear();
}

int ParametersController::numberOfRows() {
  return 1+m_law->numberOfParameter();
}

void ParametersController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (index == numberOfRows()-1) {
    return;
  }
  PointerTableCellWithEditableText * myCell = (PointerTableCellWithEditableText *) cell;
  myCell->setText(m_law->parameterNameAtIndex(index));
  FloatParameterController::willDisplayCellForIndex(cell, index);
}

HighlightCell * ParametersController::reusableParameterCell(int index, int type) {
  assert(index >= 0);
  assert(index < 2);
  return &m_menuListCell[index];
}

int ParametersController::reusableParameterCellCount(int type) {
  return m_law->numberOfParameter();
}

float ParametersController::previousParameterAtIndex(int index) {
  assert(index >= 0);
  assert(index < 2);
  return m_previousParameters[index];
}

float ParametersController::parameterAtIndex(int index) {
  return m_law->parameterValueAtIndex(index);
}

void ParametersController::setParameterAtIndex(int parameterIndex, float f) {
  if (!m_law->authorizedValueAtIndex(f, parameterIndex)) {
    app()->displayWarning("Cette valeur est interdite !");
    return;
  }
  m_law->setParameterAtIndex(f, parameterIndex);
}

void ParametersController::buttonAction() {
  m_calculationController.setCalculationAccordingToIndex(0);
  m_calculationController.selectSubview(1);
  m_calculationController.reload();
  StackViewController * stack = stackController();
  stack->push(&m_calculationController, KDColorWhite, Palette::SubTab, Palette::SubTab);
}

}

