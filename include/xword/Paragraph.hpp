#pragma once

#include "Run.hpp"
#include "Types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace xword {

class Paragraph {
public:
    Paragraph() = default;

    // Add a simple text run
    Paragraph& addRun(const std::string& text);

    // Add a text run with style
    Paragraph& addRun(const std::string& text, const RunStyle& style);

    // Add an inline LaTeX equation run (mixed with text)
    Paragraph& addEquation(const std::string& latex);

    // Set paragraph alignment
    Paragraph& setAlignment(Alignment align);

    // Set first-line indent (twips, 1pt = 20 twips, ~480 = 2 chars at 12pt)
    Paragraph& setFirstLineIndent(int twips);
    Paragraph& setFirstLineIndentChars(double chars, int fontSizePt = 12);

    // Set paragraph spacing (twips, 1pt = 20 twips)
    Paragraph& setSpacingAfter(int twips);
    Paragraph& setSpacingBefore(int twips);

    // Internal: build XML for this paragraph
    std::string toXml() const;

private:
    enum RunKind { Text, InlineEquation };

    struct RunEntry {
        std::string content;
        RunStyle style;
        RunKind kind = Text;
    };
    std::vector<RunEntry> m_runs;
    Alignment m_alignment = Alignment::Left;
    bool m_hasAlignment = false;
    int m_firstLineIndent = -1;
    int m_spacingAfter = -1;  // -1 = inherit
    int m_spacingBefore = -1; // -1 = inherit
};

} // namespace xword
