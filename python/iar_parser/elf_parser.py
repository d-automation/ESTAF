from elftools.elf.elffile import ELFFile
from construct.lib import ListContainer
import json
import re
import sys
import argparse

class DWARFstructParser:
    def __init__(self, elf_file, TARGET, out_path):
        self.elf_file = elf_file
        self.out_path = out_path
        self.die_map = {}
        self.TARGET = TARGET
        self.struct_json = {
            "name": self.TARGET,
            "fields": [],
            "size": 0
        }
        self._build_die_map()

    def _map_type(self, size):
        if size == 1:
            return "uint8"
        if size == 2:
            return "uint16"
        if size == 4:
            return "uint32"
        if size == 8:
            return "uint64"
        return f"bytes[{size}]"

    def _build_die_map(self):
        try:
            with open(self.elf_file, "rb") as f:
                elf = ELFFile(f)
                if not elf.has_dwarf_info():
                    raise ValueError(f"File {self.elf_file} not contains DWARF-info")

                dwar_finfo = elf.get_dwarf_info()
                for cu in dwar_finfo.iter_CUs():
                    for die in cu.iter_DIEs():
                        self.die_map[die.offset] = die

        except FileNotFoundError:
            raise FileNotFoundError(f"file not found in path: {self.elf_file}")
        except Exception as e:
            raise RuntimeError(f"error while elf-file reading: {str(e)}")

    def _get_name(self, die):
        a = die.attributes.get("DW_AT_name")
        if not a:
            return None
        v = a.value
        if isinstance(v, bytes):
            return v.decode(errors="ignore")
        return str(v)

    def _get_type_offset(self, die):
        a = die.attributes.get("DW_AT_type")
        if not a:
            return None

        ref = a.value

        # absolute ref
        if ref in self.die_map:
            return ref

        # CU relative ref
        ref2 = ref + die.cu.cu_offset
        if ref2 in self.die_map:
            return ref2

        return None

    def _resolve(self, die):
        if die is None:
            return None

        while die.tag in (
                "DW_TAG_typedef",
                "DW_TAG_const_type",
                "DW_TAG_volatile_type",
                "DW_TAG_restrict_type"
        ):
            ref = self._get_type_offset(die)

            if ref is None:
                return die

            target = self.die_map.get(ref)

            if target is None:
                return die

            die = target

        return die

    def _get_size(self, die):
        a = die.attributes.get("DW_AT_byte_size")
        if not a:
            return 0
        return int(a.value)

    def _get_member_offset(self, die):
        a = die.attributes.get("DW_AT_data_member_location")
        if not a:
            return 0
        v = a.value
        # ordinary int
        if isinstance(v, int):
            return v
        # ListContainer
        try:
            # [DWARFExprOp(... args=[11])]
            for item in v:
                if hasattr(item, "args"):
                    if len(item.args):
                        return int(item.args[0])
            # [35, 11]
            vals = list(v)
            if len(vals) >= 2 and isinstance(vals[1], int):
                return vals[1]
        except:
            raise RuntimeError(f"unsupported type found in structure {self.TARGET}: {str(v)}")
        return 0

    def _resolve_array(self, die):

        if die.tag != "DW_TAG_array_type":
            return None, None

        element_type = None
        count = None

        # search subrange
        for child in die.iter_children():

            if child.tag != "DW_TAG_subrange_type":
                continue

            # COUNT
            if "DW_AT_count" in child.attributes:
                count = child.attributes["DW_AT_count"].value

            elif "DW_AT_upper_bound" in child.attributes:
                # upper_bound = N-1
                count = child.attributes["DW_AT_upper_bound"].value + 1

        # base type array
        t = die.attributes.get("DW_AT_type")

        if t:
            element_type = self.die_map[t.value + die.cu.cu_offset]

        return element_type, count

    def _resolve(self, die):
        while die.tag in ("DW_TAG_typedef", "DW_TAG_const_type", "DW_TAG_volatile_type", "DW_TAG_restrict_type"):
            ref = self._get_type_offset(die)
            if ref is None:
                return die
            die = self.die_map[ref]

        return die

    def _detect_type(self, die, size):
        die = self._resolve(die)

        if die is None:
            return f"bytes[{size}]"

        if die.tag == "DW_TAG_enumeration_type":
            return "enum"

        if die.tag == "DW_TAG_pointer_type":
            return "pointer"

        if die.tag == "DW_TAG_base_type":
            name = self._get_name(die)

            if name:
                return name

        return self._map_type(size)
    def _dump_struct(self, die, base=0, prefix=""):
        die = self._resolve(die)
        if die.tag != "DW_TAG_structure_type":
            return

        for child in die.iter_children():
            if child.tag != "DW_TAG_member":
                continue

            field_name = self._get_name(child)
            field_offset = self._get_member_offset(child)

            tref = self._get_type_offset(child)
            if tref is None:
                continue

            target_die = self.die_map.get(tref)

            if target_die is None:
                print(
                    f"WARNING: type offset not found "
                    f"(field={field_name}, tref={tref})"
                )
                continue

            tdie = self._resolve(target_die)

            if tdie is None:
                print(
                    f"WARNING: unresolved type "
                    f"(field={field_name})"
                )
                continue

            size = self._get_size(tdie)

            fullname = prefix + field_name

            if tdie.tag == "DW_TAG_array_type":

                elem_die, count = self._resolve_array(tdie)

                if elem_die is None:
                    continue

                elem_die = self._resolve(elem_die)
                elem_size = self._get_size(elem_die)

                if count is None:
                    count = 1

                total_size = elem_size * count

                elem_type = self._detect_type(tdie, elem_size)

                self.struct_json["fields"].append({
                    "name": fullname,
                    "offset": base + field_offset,
                    "size": total_size,
                    "array": True,
                    "count": count,
                    "elem_size": elem_size,
                    "elem_type": elem_type
                })

                continue

            self.struct_json["fields"].append({
                "name": fullname,
                "offset": base + field_offset,
                "size": size,
                "type": self._detect_type(tdie, size)

            })

            self.struct_json["size"] = max(
                self.struct_json["size"],
                base + field_offset + size
            )

            # recursion nested struct
            if tdie.tag == "DW_TAG_structure_type":
                self._dump_struct(
                    tdie,
                    base + field_offset,
                    fullname + "."
                )

    def get_struct(self, use_typedef=True):
        find_flag = False

        for die in self.die_map.values():
            if die.tag != "DW_TAG_typedef":
                continue

            name = self._get_name(die)
            if name != self.TARGET:
                continue
            else:
                find_flag = True
                ref = self._get_type_offset(die)
                real = self.die_map[ref]
                self._dump_struct(real)
                break

        if find_flag == True:
            return self.struct_json
        else:
            raise RuntimeError(f"struct {self.TARGET} not found in elf-file")

    def save_struct_to_json(self, struct_json):
        output_file = f"{self.out_path}{self.TARGET}.json"

        with open(output_file, "w", encoding="utf-8") as f:
            json.dump(struct_json, f, indent=2, ensure_ascii=False)
            print("\nJSON saved to:", output_file)


if __name__ == "__main__":
    # TARGET = "UART_cmdSetNANDsettings_t"
    # fname = r"C:\Danila\work\sputnik_test\src\sputnik\Debug\c.out"
    # usage example:
    # python elf_parser.py -f "C:\Danila\work\sputnik_test\src\sputnik\Debug\c.out" -t "Snapshot_HK_t"

    # argparser = argparse.ArgumentParser("elf_parser.py")
    #
    # argparser.add_argument('-f', '--elf_file', required=True,
    #                        help='specify path to elf-file')
    # argparser.add_argument('-t', '--struct_name', required=True,
    #                        help='specify structure name (\"UART_t\" for example')
    # argparser.add_argument('-o', '--out_path', required=True,
    #                        help='specify out path with map data file in json format (\"C:/data/\" for example)')
    #
    # args = argparser.parse_args()

    try:
        elf_file = "C:/Danila/work/1892VM12/boot2/MultiCore_Configuration_Debug/boot2.elf"
        struct_name = "BuffU8Type"
        out_path = "./"
        parser = DWARFstructParser(elf_file, struct_name, out_path)

        # parser = DWARFstructParser(args.elf_file, args.struct_name, args.out_path)
        json_struct = parser.get_struct()
        parser.save_struct_to_json(json_struct)

    except (FileNotFoundError, ValueError, LookupError, IOError) as err:
        print(f"ERROR code 2: {str(err)}", file=sys.stderr)
        sys.exit(2)

    except Exception as unexpected_arr:
        print(f"ERROR code 3: {str(unexpected_arr)}", file=sys.stderr)
        sys.exit(3)