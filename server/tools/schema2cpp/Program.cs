using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace schema2cpp
{
    class Program
    {
        static Dictionary<string, string> ParseArgs(string[] args)
        {
            static bool isArg(string s) => s.StartsWith('-') || s.StartsWith('/');
            var dict = new Dictionary<string, string>();
            for (int i = 0; i < args.Length; i++)
            {
                if (isArg(args[i]))
                {
                    if (i + 1 < args.Length && isArg(args[i + 1]))
                    {
                        dict[args[i].TrimStart('-', '/')] = args[++i];
                    }
                    else
                    {
                        dict[args[i].TrimStart('-', '/')] = String.Empty;
                    }
                }
            }
            return dict;
        }
        static int Main(string[] args)
        {
            var dict = ParseArgs(args);
            bool errflag = false;
            if (!dict.TryGetValue("i", out var input))
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Expected input arg `-i PATH` to be present");
                Console.ResetColor();
                errflag = true;
            }
            else if (!File.Exists(input))
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"Could not locate input file `{input}`.");
                Console.ResetColor();
                errflag = true;
            }
            if (!dict.TryGetValue("o", out var output))
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Expected output arg `-o PATH` to be present");
                Console.ResetColor();
                errflag = true;
            }
            else
            {
                Directory.CreateDirectory(Path.GetDirectoryName(output));
            }
            if (errflag)
            {
                return -1;
            }

            using (var writer = new StreamWriter(output))
            {
                var schema2cpp = new SchemaToCpp(File.ReadAllText(input), writer);
            }
            return 0;
        }
    }
}
