
import shell from "shelljs";
import path from "path";

shell.mkdir("-p", "dist/public");
shell.cp("-R", "libtorch", "dist/");
shell.cp("-R", "data", "dist/");
shell.mkdir("-p", "dist/public");
shell.cp("-R", "src/browser/*", "dist/public");

