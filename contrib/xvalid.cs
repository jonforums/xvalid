/**
 * Author: Jon Maken
 * Revision: 12/02/2010 8:41:04 PM
 * License: Modified BSD License (3-clause)
 */
using System;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Schema;

namespace XValid
{
	/// <summary>
	/// XCheck exit codes
	/// </summary>
	enum ExitCode
	{
		OK = 0,				// XML is well-formed and valid
		CmdErr = -1,		// incorrect command line invocation
		IOErr = -2,			// I/O error
		ParseErr = -3,		// XML parsing error
		ValErr = -4,		// Validation error
	}

	internal struct XValidContext
	{
		internal string xmlFile;
		internal string dtdFile;
		internal string xmlRoot;
		internal string xsdFile;
		internal bool error;

		public override string ToString()
		{
			string s = String.Format(
						"<#XValidContext => xmlFile:{0}, dtdFile:{1}, xmlRoot:{2}, xsdFile:{3}>",
						xmlFile,
						dtdFile,
						xmlRoot,
						xsdFile);

			return s;
		}
	}

	/// <summary>
	/// <para>Console app that checks if the specified XML file is well formed.
	/// If the XML file has an embedded DTD/XSD schema reference or a DTD/XSD
	/// schema location is given on the command line, the XML file will be
	/// validated against the specified schema.</para>
	/// <para>NOTE: only a DTD or an XSD location can be provided as a command
	/// line argument; both can not be specified.
	/// </para>
	/// <example>Example command line invocations:
	/// Well-formed? <c>xval test.xml</c>
	/// Well-formed and valid (DTD)? <c>xval --dtd valid.dtd RootElement test.xml</c>
	/// Well-formed and valid (XSD)? <c>xval --xsd valid.xsd test.xml</c>
	/// </example>
	/// <returns>simple error message to StdErr and integer exit code.</returns>
	/// </summary>
	class XValid
	{
		private static string _VERSION = "0.5.0";
		private static XValidContext _context = new XValidContext();
		private static int _start;
		private static XmlParserContext _parserCtx;

		public static int Main(string[] args)
		{
			if (args.Length < 1)
			{
				Console.WriteLine(Usage());
				return (int) ExitCode.CmdErr;
			}
			ParseArgs(args, out _start);
			CheckConfig();

			// configure the XmlReader for validation
			XmlReaderSettings settings = new XmlReaderSettings();
			settings.ValidationEventHandler += new ValidationEventHandler(ValidationCallback);

			if (_context.xsdFile != null)
			{
				// add specified schema to the reader's XmlSchemaCollection using the
				// namespace defined in the targetNamespace attribute of the schema by
				// passing null as 1st arg to Add()
				XmlSchemaSet sc = new XmlSchemaSet();
				sc.Add(null, _context.xsdFile);

				settings.ValidationType = ValidationType.Schema;
				settings.Schemas = sc;
				settings.ValidationFlags |= XmlSchemaValidationFlags.ReportValidationWarnings;
			}
			else if (_context.dtdFile != null)
			{
				InitParserContext(_context.xmlRoot, _context.dtdFile, out _parserCtx);
			}

			// validate the files
			for (int i = _start; i < args.Length; i++)
			{
				string f = _context.xmlFile = args[i];

				if (!File.Exists(f))
				{
					Console.WriteLine("File {0} does not exist", f);
					continue;
				}

				Console.WriteLine("Processing {0}...", f);

				// TODO replace obsolete XmlValidatingReader with XmlReader?
				if (_context.dtdFile != null)
				{
					using (FileStream s = new FileStream(f, FileMode.Open, FileAccess.Read))
					{
						using (XmlValidatingReader vr = new XmlValidatingReader(s, XmlNodeType.Document, _parserCtx))
						{
							vr.ValidationType = ValidationType.Auto;
							vr.ValidationEventHandler += new ValidationEventHandler(ValidationCallback);

							while (vr.Read());
						}
					}
				}
				else if (_context.xsdFile != null)
				{
					using (XmlReader r = XmlReader.Create(f, settings))
						while (r.Read());
				}
				else
				{
					using (XmlTextReader tr = new XmlTextReader(f))
					{
						using (XmlValidatingReader vr = new XmlValidatingReader(tr))
						{
							vr.ValidationType = ValidationType.Auto;
							vr.ValidationEventHandler += new ValidationEventHandler(ValidationCallback);

							while (vr.Read());
						}
					}
				}
			}

			return (int) ExitCode.OK;
		}

		private static void ValidationCallback(object sender, ValidationEventArgs e)
		{
			XmlSchemaException xse = e.Exception;

			if (e.Severity == XmlSeverityType.Warning)
			{
				Console.Error.WriteLine("VALIDATION WARNING:\n[{0}:{1}] {2}",
										xse.LineNumber,
										xse.LinePosition,
										xse.Message);
			}
			else
			{
				//valErrFlag = true;
				Console.Error.WriteLine("VALIDATION ERROR:\n[{0}:{1}] {2}",
										xse.LineNumber,
										xse.LinePosition,
										xse.Message);
			}
		}

		private static string Usage()
		{
			StringBuilder sb = new StringBuilder();
			sb.AppendLine(String.Format("XValid {0} - validate XML documents", _VERSION));
			sb.AppendLine("Usage: xval [options] XML_FILE ...\n");
			sb.AppendLine("where validation options are:");
			sb.AppendLine("  --dtd FILE ROOT  validate with DTD FILE at ROOT element");
			sb.AppendLine("  --xsd FILE       validate with XSD FILE");

			return sb.ToString();
		}

		private static void ParseArgs(string[] args, out int start)
		{
			int i;

			for (i = 0; i < args.Length; i++)
			{
				if (args[i].Equals("-dtd") || args[i].Equals("--dtd"))
				{
					i++;
					_context.dtdFile = args[i];
					i++;
					_context.xmlRoot = args[i];
					continue;
				}
				else if (args[i].Equals("-xsd") || args[i].Equals("--xsd"))
				{
					i++;
					_context.xsdFile = args[i];
					continue;
				}
				else break;
			}

			start = i;
		}

		private static void CheckConfig()
		{
			if ((_context.dtdFile != null) && (_context.xsdFile != null))
				throw new ArgumentException("choose one of --dtd or --xsd, not both");
			if ((_context.dtdFile != null) && (_context.xmlRoot == null))
				throw new ArgumentException("must provide both a DTD file and a root XML element");
			if ((_context.dtdFile != null) && !File.Exists(_context.dtdFile))
				throw new ArgumentException("given DTD file doesn't exist");
			if ((_context.xsdFile != null) && !File.Exists(_context.xsdFile))
				throw new ArgumentException("given XSD file doesn't exist");
		}

		// build context for parser in order to validate with the specified DTD
		private static void InitParserContext(
								string xmlRoot,
								string dtdFile,
								out XmlParserContext ctx)
		{
			ctx = new XmlParserContext(
						null,				// XmlNameTable
						null,				// XmlNamespaceManager
						xmlRoot,			// docTypeName
						String.Empty,		// pubId
						dtdFile,			// sysId
						String.Empty,		// internalSubset
						String.Empty,		// baseURI
						String.Empty,		// xmlLang
						XmlSpace.Default,	// xmlSpace
						Encoding.UTF8);		// Encoding
		}

	}
}
