#include "xword/xword.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
    using namespace xword;
    const std::string aPath = "test_merge_a.docx";
    const std::string bPath = "test_merge_b.docx";

    Document a;
    a.addParagraph("A paragraph");
    assert(a.save(aPath));
    Document b;
    b.addParagraph().addRun("B paragraph", RunStyle().bold());
    assert(b.save(bPath));

    Document sourceFormat;
    assert(sourceFormat.open(aPath));
    Document source;
    assert(source.open(bPath));
    assert(sourceFormat.appendDocument(source, MergeFormat::Source));
    assert(sourceFormat.save("test_merge_source.docx"));

    Document targetFormat;
    assert(targetFormat.open(aPath));
    assert(targetFormat.appendDocument(source, MergeFormat::Target));
    assert(targetFormat.save("test_merge_target.docx"));

    std::cout << "Document merge object overload passed\n";
    return 0;
}
