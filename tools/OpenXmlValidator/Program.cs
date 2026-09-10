using System;
using System.IO;
using System.Linq;
using DocumentFormat.OpenXml;
using DocumentFormat.OpenXml.Packaging;
using DocumentFormat.OpenXml.Validation;

internal static class Program
{
    private static int Main(string[] args)
    {
        if (args.Length == 0) { Console.Error.WriteLine("Usage: OpenXmlValidator file.docx [file.docx ...]"); return 2; }
        int count = 0;
        foreach (var path in args)
        {
            try
            {
                using var document = WordprocessingDocument.Open(path, false);
                var errors = new OpenXmlValidator(FileFormatVersions.Office2019).Validate(document).ToList();
                foreach (var error in errors)
                    Console.Error.WriteLine($"{path}: {error.Part?.Uri} {error.Path?.XPath}: {error.Description}");
                Console.WriteLine($"{Path.GetFileName(path)}: {errors.Count} Open XML errors");
                count += errors.Count;
            }
            catch (Exception error) { Console.Error.WriteLine($"{path}: {error.Message}"); count++; }
        }
        return count == 0 ? 0 : 1;
    }
}
