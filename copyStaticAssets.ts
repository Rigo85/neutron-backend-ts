
import shell from "shelljs";
import path from "path";

shell.mkdir("-p", "dist/public");
shell.cp("-R", "libtorch", "dist/");
shell.cp("-R", "data", "dist/");
shell.mkdir("-p", "dist/public");
shell.cp("-R", "src/browser/*", "dist/public");

shell.mkdir("-p", "dist/native/build/Release");
shell.mkdir("-p", "dist/native/rl/build/Release");

const minimaxAddon = path.join("native", "build", "Release", "neutron_minimax.node");
const rlAddon = path.join("native", "rl", "build", "Release", "neutron_rl_addon.node");

if (shell.test("-f", minimaxAddon)) {
	shell.cp(minimaxAddon, path.join("dist", "native", "build", "Release"));
}

if (shell.test("-f", rlAddon)) {
	shell.cp(rlAddon, path.join("dist", "native", "rl", "build", "Release"));
}
