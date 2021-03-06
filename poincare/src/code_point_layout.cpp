#include <poincare/code_point_layout.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>

namespace Poincare {

// LayoutNode
void CodePointLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (cursor->position() == LayoutCursor::Position::Right) {
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  LayoutNode * parentNode = parent();
  if (parentNode != nullptr) {
    parentNode->moveCursorLeft(cursor, shouldRecomputeLayout);
  }
}

void CodePointLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (cursor->position() == LayoutCursor::Position::Left) {
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  LayoutNode * parentNode = parent();
  if (parentNode != nullptr) {
    parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
  }
}

int CodePointLayoutNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::CodePoint(buffer, bufferSize, m_codePoint);
}

bool CodePointLayoutNode::isCollapsable(int * numberOfOpenParenthesis, bool goingLeft) const {
  if (*numberOfOpenParenthesis <= 0) {
    if (m_codePoint == '+'
        || m_codePoint == UCodePointRightwardsArrow
        || m_codePoint == '='
        || m_codePoint == ',')
    {
      return false;
    }
    if (m_codePoint == '-') {
      /* If the expression is like 3ᴇ-200, we want '-' to be collapsable.
       * Otherwise, '-' is not collapsable. */
      Layout thisRef = CodePointLayout(this);
      Layout parent = thisRef.parent();
      if (!parent.isUninitialized()) {
        int indexOfThis = parent.indexOfChild(thisRef);
        if (indexOfThis > 0) {
          Layout leftBrother = parent.childAtIndex(indexOfThis-1);
          if (leftBrother.type() == Type::CodePointLayout
              && static_cast<CodePointLayout&>(leftBrother).codePoint() == UCodePointLatinLetterSmallCapitalE)
          {
            return true;
          }
        }
      }
      return false;
    }
  }
  return true;
}

bool CodePointLayoutNode::canBeOmittedMultiplicationLeftFactor() const {
  if (isMultiplicationCodePoint()) {
    return false;
  }
  return LayoutNode::canBeOmittedMultiplicationRightFactor();
}

bool CodePointLayoutNode::canBeOmittedMultiplicationRightFactor() const {
  if (m_codePoint == '!' || isMultiplicationCodePoint()) {
    return false;
  }
  return LayoutNode::canBeOmittedMultiplicationRightFactor();
}

// Sizing and positioning
KDSize CodePointLayoutNode::computeSize() {
  return m_font->glyphSize();
}

KDCoordinate CodePointLayoutNode::computeBaseline() {
  return m_font->glyphSize().height()/2;
}

void CodePointLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  constexpr int bufferSize = sizeof(CodePoint)/sizeof(char) + 1; // Null-terminating char
  char buffer[bufferSize];
  SerializationHelper::CodePoint(buffer, bufferSize, m_codePoint);
  ctx->drawString(buffer, p, m_font, expressionColor, backgroundColor);
}

bool CodePointLayoutNode::isMultiplicationCodePoint() const {
  return m_codePoint == '*'
    || m_codePoint == UCodePointMultiplicationSign
    || m_codePoint == UCodePointMiddleDot;
}

bool CodePointLayoutNode::protectedIsIdenticalTo(Layout l) {
  assert(l.type() == Type::CodePointLayout);
  CodePointLayout & cpl = static_cast<CodePointLayout &>(l);
  return codePoint() == cpl.codePoint() && font() == cpl.font();
}

CodePointLayout CodePointLayout::Builder(CodePoint c, const KDFont * font) {
  void * bufferNode = TreePool::sharedPool()->alloc(sizeof(CodePointLayoutNode));
  CodePointLayoutNode * node = new (bufferNode) CodePointLayoutNode(c, font);
  TreeHandle h = TreeHandle::BuildWithGhostChildren(node);
  return static_cast<CodePointLayout &>(h);
}

}
