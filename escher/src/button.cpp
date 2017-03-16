#include <escher/button.h>
#include <escher/palette.h>
#include <assert.h>

Button::Button(Responder * parentResponder, const char * textBody, Invocation invocation, KDText::FontSize size, KDColor textColor) :
  HighlightCell(),
  Responder(parentResponder),
  m_pointerTextView(PointerTextView(size, textBody, 0.5f, 0.5f, textColor)),
  m_invocation(invocation)
{
}

int Button::numberOfSubviews() const {
  return 1;
}

View * Button::subviewAtIndex(int index) {
  assert(index == 0);
  return &m_pointerTextView;
}

void Button::layoutSubviews() {
  m_pointerTextView.setFrame(bounds());
}

bool Button::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK) {
    m_invocation.perform(this);
    return true;
  }
  return false;
}

void Button::setHighlighted(bool highlight) {
  HighlightCell::setHighlighted(highlight);
  KDColor backgroundColor = highlight? Palette::Select : KDColorWhite;
  m_pointerTextView.setBackgroundColor(backgroundColor);
  markRectAsDirty(bounds());
}

KDSize Button::minimalSizeForOptimalDisplay() const {
  KDSize textSize = m_pointerTextView.minimalSizeForOptimalDisplay();
  return KDSize(textSize.width() + k_horizontalMargin, textSize.height() + k_verticalMargin);
}
