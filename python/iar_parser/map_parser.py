import json
import re
import sys
import argparse

class map_parser:
    def __init__(self, map_file, out_path):
        self.map_file = map_file
        self.out_path = out_path
        self.map_data = None
        self._open_map_file()

    def _open_map_file(self):
        try:
            with open(self.map_file, 'r', encoding="utf-8") as f:
                self.map_data = f.read()
        except FileNotFoundError:
            raise FileNotFoundError(f"file not found in path: {self.map_file}")
        except Exception as e:
            raise RuntimeError(f"error while .map file reading: {str(e)}")

    def get_description_to_json(self, descript_name):
        entries = {}

        lines = self.map_data.splitlines()

        if "ENTRY LIST" in descript_name:
            section_lines = []
            inside_entry_list = False

            for line in lines:
                if "ENTRY LIST" in line:
                    inside_entry_list = True
                    continue

                if inside_entry_list:
                    if "********" in line:
                        break
                    section_lines.append(line)

            if not inside_entry_list:
                raise RuntimeError(f"specified description name: {descript_name} not found in specified .map file")

            # 2. Регулярное выражение для разбора строк (символы ?, $, . внутри \S+)
            pattern = re.compile(
            r"^\s*(?P<name>\S+)\s+(?P<address>0x[0-9a-fA-F]+)\s+(?P<size>0x[0-9a-fA-F]+|--)\s+(?P<type>\w+)"
            )

            for line in section_lines:
                line = line.strip()
                match = pattern.match(line)
                if match:
                    data = match.groupdict()
                    name = data["name"]
                    raw_size = data["size"]

                    size_val = None if raw_size == "--" else int(raw_size, 16)

                    entries[name] = {
                        "address": data["address"],
                        "size_hex": raw_size if raw_size != "--" else None,
                        "size_bytes": size_val,
                        "type": data["type"],
                    }

            return entries

        else:
            raise RuntimeError(f"specified description name: {descript_name} not found in .map file or not supported")

    def save_data_to_json(self, data_json, descript_name):
        output_file = f"{self.out_path}{descript_name.replace(" ", "_")}_map.json"

        with open(output_file, "w", encoding="utf-8") as f:
            json.dump(data_json, f, indent=2, ensure_ascii=False)
            print("\nJSON saved to:", output_file)


if __name__ == "__main__":
    #descript_name = "ENTRY LIST"
    #fname = r"C:\Danila\work\sputnik_test\src\sputnik\Debug\sputnik.map"
    # usage example:
    # python elf_parser.py -f "C:\Danila\work\sputnik_test\src\sputnik\Debug\sputnik.map" -s "ENTRY LIST"

    argparser = argparse.ArgumentParser("map_parser.py")

    argparser.add_argument('-f', '--map_file', required=True,
                           help='specify path to map-file')
    argparser.add_argument('-s', '--section_name', required=True,
                           help='specify section name in .map file (\"ENTRY LIST\" for example')
    argparser.add_argument('-o', '--out_path', required=True,
                           help='specify out path with map data file in json format (\"C:/data/\" for example)')

    args = argparser.parse_args()

    try:
        parser = map_parser(args.map_file, args.out_path)
        data_json = parser.get_description_to_json(args.section_name)
        parser.save_data_to_json(data_json, args.section_name)

    except (FileNotFoundError, ValueError, LookupError, IOError) as err:
        print(f"ERROR code 2: {str(err)}", file=sys.stderr)
        sys.exit(2)

    except Exception as unexpected_arr:
        print(f"Error code 3: {str(unexpected_arr)}", file=sys.stderr)
        sys.exit(3)

# def parse_iar_map_text(map_text):
#     entries = {}
#
#     lines = map_text.splitlines()
#     section_lines = []
#     inside_entry_list = False
#
#     for line in lines:
#         # Если встретили заголовок (независимо от количества звездочек и пробелов)
#         if "ENTRY LIST" in line:
#             inside_entry_list = True
#             continue  # Пропускаем саму строку заголовка
#
#         if inside_entry_list:
#             # Если дошли до следующей крупной секции (строка из множества звезд) — выходим
#             if "********" in line:
#                 break
#             section_lines.append(line)
#
#     if not inside_entry_list:
#         print("Ошибка: Секция ENTRY LIST не найдена в файле.")
#         return entries
#
#     # 2. Регулярное выражение для разбора строк (символы ?, $, . внутри \S+)
#     pattern = re.compile(
#         r"^\s*(?P<name>\S+)\s+(?P<address>0x[0-9a-fA-F]+)\s+(?P<size>0x[0-9a-fA-F]+|--)\s+(?P<type>\w+)"
#     )
#
#     # 3. Парсим собранные строки
#     for line in section_lines:
#         line = line.strip()
#         match = pattern.match(line)
#         if match:
#             data = match.groupdict()
#             name = data["name"]
#             raw_size = data["size"]
#
#             size_val = None if raw_size == "--" else int(raw_size, 16)
#
#             entries[name] = {
#                 "address": data["address"],
#                 "size_hex": raw_size if raw_size != "--" else None,
#                 "size_bytes": size_val,
#                 "type": data["type"],
#             }
#
#     return entries
#
# fname_map = r"C:\Danila\work\sputnik_test\src\sputnik\Debug\sputnik.map"
#
# with open(fname_map, 'r', encoding="utf-8") as f:
#     data = f.read()
#     # print(data)
#
#     result_json = parse_iar_map_text(data)
#
#     json_map_file = "map.json"
#     with open(json_map_file, 'w', encoding='utf-8') as f:
#         # Выводим в stdout готовый JSON (Qt-стенд сможет его легко прочесть)
#         json.dump(result_json, f, indent=2, ensure_ascii=False)