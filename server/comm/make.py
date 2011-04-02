import os

COMPILE_LINE = """g++ -Wall -g
-I/usr/include
-I/usr/local/include
-I/usr/local/BerkeleyDB.4.7/include
-I/Life/360io/server/shared/src
""".replace("\n", " ")[:-1]

optional = ""
if os.__file__.find("/Library/Frameworks") < 0:
    optional = "-luuid"
    
LINK_LINE = ("""g++ -Wall -g
-L/usr/lib
-L/usr/local/lib
-L/usr/local/BerkeleyDB.4.7/lib
-lev
-ldb_cxx-4.7
-lpthread
%s
build/*.o
-o build/360comm
""" % (optional)).replace("\n", " ")

def walk(ctx, dir, files):
    for file in files:
        if dir == os.path.join("Core", "Test"):
            continue

        path = os.path.join(dir, file)
        if os.path.isfile(path) and os.path.splitext(path)[1] == ".cpp":
            ctx.append(path)

cfiles = []
os.path.walk("/Life/360io/server/shared/src/", walk, cfiles)
os.path.walk("src", walk, cfiles)
ofiles = [os.path.join("build", os.path.split(file.replace(".cpp", ".o"))[1]) for file in cfiles]
compile_lines = [COMPILE_LINE + " -c " + cfile + " -o " + ofile for (cfile, ofile) in zip(cfiles, ofiles)]

os.system("rm build/*")

for line in compile_lines:
    print line[len(COMPILE_LINE):]
    os.system(line)

print LINK_LINE
os.system(LINK_LINE)

