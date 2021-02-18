using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace schema2cpp
{
    public class SchemaToCpp
    {
        private JObject schema;
        private StreamWriter writer;

        public bool Error { get; private set; }

        public SchemaToCpp(string v, StreamWriter writer)
        {
            this.schema = Newtonsoft.Json.Linq.JObject.Parse(v);
            this.writer = writer;
        }


        private void FileStart()
        {
            const string @namespace = "dsp::data";
            Console.WriteLine($"Writing out FileHead with namespace `{@namespace}`.");
            this.writer.WriteLine(@"#pragma once");
            this.writer.WriteLine(@"#include <string>");
            this.writer.WriteLine(@"#include <vector>");
            this.writer.WriteLine(@"#include <nlohman/json>");
            this.writer.WriteLine();
            this.writer.WriteLine($@"namespace {@namespace}");
            this.writer.WriteLine(@"{");
        }
        private void FileEnd()
        {
            Console.WriteLine($"Writing out FileEnd.");
            this.writer.WriteLine(@"}");
        }
        private static string tab(int index)
        {
            return new string(' ', 4 * index);
        }
        private void ForwardDeclarations()
        {
            Console.WriteLine($"Writing out Forward Declarations.");
            foreach (var kvp in this.schema)
            {
                Console.WriteLine($"{tab(1)}{kvp.Key}");
                this.writer.WriteLine($@"{tab(1)}class {kvp.Key};");
            }
        }

        public void ParseSchema()
        {
            this.FileStart();
            this.ForwardDeclarations();
            foreach (var kvp in this.schema)
            {
                CreateDefinition(kvp.Key, (JObject)kvp.Value);
            }
            this.FileEnd();
        }
        private static string UppercaseUnderscoreSplit(string str)
        {
            var upperCaseChars = str.Skip(1).Count((c) => char.IsUpper(c));
            if (upperCaseChars == 0)
            {
                return str;
            }
            var stringBuilder = new StringBuilder(str.Length + upperCaseChars);
            str.Append(str[0]);
            foreach (var c in str.Skip(1))
            {
                if (char.IsUpper(c))
                {
                    str.Append('_');
                    str.Append(char.ToLower(c));
                }
                else
                {
                    str.Append(c);
                }
            }
            return stringBuilder.ToString();
        }
        private void CreateDefinition(string key, JObject obj)
        {
            const string key_allof = "allof";
            const string key_properties = "properties";
            const string key_required = "required";
            const string key_ref = "$ref";
            const string key_enum = "enum";
            const string key_enum_alt = "_enum";
            const string key_enumDescription = "enumDescription";
            const string key_custom_isRequired = "___isRequired";
            const string key_custom_name = "___name";
            List<JObject> resolveReferences(JObject jobj, int level)
            {
                var localProperties = new List<JObject>();
                foreach (JObject allOfObj in jobj[key_allof] as JArray)
                {
                    if (allOfObj.ContainsKey(key_ref))
                    {
                        string refpath = allOfObj[key_ref].Value<string>();
                        var res = this.schema.SelectToken(allOfObj[key_ref].Value<string>());
                        if (res is null)
                        {
                            Console.ForegroundColor = ConsoleColor.Red;
                            Console.WriteLine($"{tab(level)}Failed to resolve json-path `{refpath}`.");
                            Console.ResetColor();
                            this.Error = true;
                            continue;
                        }
                        if (!(res is JObject resJObject))
                        {
                            Console.ForegroundColor = ConsoleColor.Red;
                            Console.WriteLine($"{tab(level)}json-path `{refpath}` did not returned a JObject.");
                            Console.ResetColor();
                            this.Error = true;
                            continue;
                        }
                        Console.WriteLine($@"{tab(level)}Including `{refpath}`.");
                        localProperties.AddRange(resolveReferences(resJObject, level + 1));
                    }
                    else
                    {
                        var properties = allOfObj[key_properties] as JObject;
                        var required = (allOfObj[key_required] as JArray).Select((q) => q.Value<string>());
                        foreach ((var key, var value) in properties)
                        {
                            var propertyObj = value as JObject;
                            if (required.Contains(key))
                            {
                                propertyObj[key_custom_isRequired] = true;
                            }
                            propertyObj[key_custom_name] = key;
                            localProperties.Add(propertyObj);
                        }
                    }
                }
                return localProperties;
            }
            string enumClassName(string propertyKey) => String.Join("_", UppercaseUnderscoreSplit(key), UppercaseUnderscoreSplit(propertyKey), "kind");

            Console.WriteLine($@"Creating definition `{key}`.");
            Console.WriteLine($@"{tab(1)}Gathering all properties.");
            var allProperties = resolveReferences(obj, 2);
            Console.WriteLine($@"{tab(1)}Found {allProperties.Count} properties.");
            var enumProperties = allProperties.Where((q) => q.ContainsKey(key_enum) || q.ContainsKey(key_enum_alt)).ToArray();
            Console.WriteLine($@"{tab(1)}Found {enumProperties.Length} enums.");

            Console.WriteLine($@"{tab(1)}Writing out enums.");
            foreach (var enumProperty in enumProperties)
            {
                var enumerationsA = (enumProperty.TryGetValue(key_enum, out var ___key_enum_extract) ? ((JArray)___key_enum_extract).Select((it) => it.Value<string>()) : Array.Empty<string>());
                var enumerationsB = (enumProperty.TryGetValue(key_enum_alt, out var ___key_enum_alt_extract) ? ((JArray)___key_enum_alt_extract).Select((it) => it.Value<string>()) : Array.Empty<string>());
                var enumerations = enumerationsA.Concat(enumerationsB).ToArray();
                var enumerationDescriptions = (enumProperty.TryGetValue(key_enumDescription, out var ___key_enumDescription_extract) ? ((JArray)___key_enumDescription_extract).Select((it) => it.Value<string>()).ToArray() : Array.Empty<string>());
                this.writer.WriteLine($@"{tab(1)}enum class {enumClassName(enumProperty[key_custom_name].Value<string>())}");
                this.writer.WriteLine($@"{tab(1)}{{");
                if (enumProperty.TryGetValue(key_enum, out var enumTokens))
                {
                    for (int i = 0; i < enumerations.Length; i++)
                    {
                        if (enumerationDescriptions.Length > i)
                        {
                            this.writer.WriteLine($@"{tab(2)}/*");
                            this.writer.WriteLine($@"{tab(2)}{enumerationDescriptions[i]}");
                            this.writer.WriteLine($@"{tab(2)}*/");
                        }
                        this.writer.WriteLine($@"{tab(2)}{enumerations[i]},");
                    }
                }
                this.writer.WriteLine($@"{tab(1)}}}");
            }
        }
    }
}
