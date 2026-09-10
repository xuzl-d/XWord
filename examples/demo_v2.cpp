#include "xword/xword.hpp"
#include <iostream>

int main(int argc,char** argv) {
    using namespace xword;
    Document doc;
    DocumentProperties properties; properties.title=u8"XWord 2.0 综合示例"; properties.creator="XWord"; properties.language="zh-CN";
    doc.setProperties(properties).setCustomProperty("Revision",CustomProperty{"2",PropertyType::Number});
    doc.setBodyFont(u8"宋体","Times New Roman").enableHeadingNumbering().enableImageNumbering().enableTableNumbering();
    doc.currentSection().setTitlePage().header(HeaderFooterType::First).addParagraph(u8"封面").setAlignment(Alignment::Center);
    doc.addParagraph(u8"通用 Word 生成示例").setAlignment(Alignment::Center).setStyle(RunStyle().bold().fontSize(28));

    doc.addSection().setPageNumbering(NumberFormat::LowerRoman,1);
    doc.currentSection().footer().addParagraph().addPageNumber().setAlignment(Alignment::Center);
    doc.addTOC("1-3",u8"目录");
    doc.addParagraph(u8"目录、页码与引用由 Word 更新域后计算。");

    doc.addSection().setPageNumbering(NumberFormat::Decimal,1).setTitlePage(false);
    doc.addHeadingParagraph(u8"正文",1).setBookmark(TargetId("chapter",TargetKind::Heading));
    doc.addParagraph(u8"参见表 ").addReference(TargetId("results",TargetKind::Table),ReferenceKind::Number).addRun(u8"，位于第 ").addReference(TargetId("results",TargetKind::Table),ReferenceKind::Page).addRun(u8" 页。");
    auto& note=doc.addFootnote(); note.addParagraph(u8"这是富文本脚注。").addRun(u8"可加粗",RunStyle().bold());
    doc.addParagraph(u8"脚注示例").addFootnoteRef(note.id()).addRun(u8"；尾注示例").addEndnoteRef(doc.addEndnote(u8"这是尾注。"));
    auto comment=doc.addComment(u8"请核对数据。",u8"审核人");
    doc.addParagraph().startComment(comment).addRun(u8"需要审核的段落").endComment(comment);

    Page landscape; landscape.setOrientation(Orientation::Landscape);
    doc.addSection().setPage(landscape);
    auto& table=doc.addTable(25,3); table.setBorderStyle(TableStyle::Grid).setHeaderRows(1).setCaption(u8"计算结果").setBookmark(TargetId("results",TargetKind::Table));
    for(int r=0;r<table.rows();++r) { table.setRowAllowSplit(r,false); for(int c=0;c<table.cols();++c)table.cell(r,c).addParagraph(std::to_string(r)+" / "+std::to_string(c)); }
    if(argc>2)doc.addImage(argv[2]).setSize(600,0).setCaption(u8"示例图片").setAltText(u8"由命令行提供的示例图片");

    doc.addSection().setPage(Page()).setColumns(2,Length::cm(1));
    doc.addHeading(u8"双栏内容",1);
    doc.addParagraph(u8"第一栏内容").addBreak(BreakType::Column).addRun(u8"下一栏内容");
    doc.addSection(SectionBreakType::NextColumn).setColumns(2);
    doc.addParagraph(u8"从下一栏开始的新节。");

    doc.addSection().setColumns(1);
    BibliographySource source; source.tag="OpenXML"; source.type=SourceType::InternetSite; source.title="Office Open XML"; source.year="2026"; source.url="https://learn.microsoft.com/office/open-xml/"; source.authors.push_back({"","","","Microsoft"});
    doc.addSource(source);
    doc.addParagraph(u8"规范参考：").addCitation("OpenXML");
    doc.addBibliography(u8"参考文献");
    auto result=doc.saveDetailed(argc>1?argv[1]:"xword_v2.docx");
    if(!result.success) {std::cerr<<result.error.part<<": "<<result.error.message<<'\n';return 1;}
    for(const auto& warning:result.warnings)std::cerr<<warning.message<<'\n';
    return 0;
}
