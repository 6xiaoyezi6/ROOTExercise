"""功能：执行修复归一化后的两个 EnergyLoss Notebook，保存模型及修复前后比较。
方法：逐单元执行原 Notebook，捕获文本和 matplotlib 图像；用同一数据和 seed=42 划分复算两版模型。
注意事项：固定原有 2000 轮和最终权重，不选择最优测试权重；CPU 单线程，float64。
仅更新 EnergyLoss 的 Notebook 和模型；旧文件已备份在本目录，原始 ROOT 和 SSD 子目录不修改。
"""

import ast
import base64
import contextlib
import io
import json
import os
from pathlib import Path
import sys
import time

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import torch
import uproot

RUN_DIR = Path(__file__).resolve().parent
ENERGY_DIR = RUN_DIR.parent.parent
os.chdir(ENERGY_DIR)
torch.set_num_threads(1)
torch.set_num_interop_threads(1)


def load_data(checkpoint):
    datasets = []
    for filename in ["Processd.root", "Processd_test.root"]:
        names = checkpoint["feature_names"] + ["KinematicsAtVertex"]
        with uproot.open(filename) as root_file:
            arrays = root_file["tree"].arrays(names, library="np")
        x = np.column_stack([arrays[n] for n in checkpoint["feature_names"]])
        y = arrays["KinematicsAtVertex"].reshape(-1, 1)
        datasets.append((x, y))
    x, y = datasets[0]
    index = torch.randperm(len(y), generator=torch.Generator().manual_seed(42)).numpy()
    cut = int(0.8 * len(y))
    return {"training": (x[index[:cut]], y[index[:cut]]),
            "internal_test": (x[index[cut:]], y[index[cut:]]),
            "external_test": datasets[1]}


def evaluate(checkpoint, datasets):
    model = checkpoint["model"].to(dtype=torch.float64).eval()
    metrics, residuals = {}, {}
    for name, (x, y) in datasets.items():
        scaled = (x - checkpoint["x_mean"]) / checkpoint["x_std"]
        with torch.no_grad():
            pred = model(torch.from_numpy(np.ascontiguousarray(scaled))).numpy()
        pred = pred * checkpoint["y_std"] + checkpoint["y_mean"]
        residual = (pred - y).ravel()
        residuals[name] = residual
        metrics[name] = {
            "events": len(y), "RMSE_MeV": float(np.sqrt(np.mean(residual**2))),
            "MAE_MeV": float(np.mean(abs(residual))), "Bias_MeV": float(residual.mean()),
            "ResidualStd_MeV": float(residual.std()),
            "R2": float(1 - np.mean(residual**2) / y.var()),
            "AbsErrorQ95_MeV": float(np.quantile(abs(residual), 0.95)),
            "MaxAbsError_MeV": float(abs(residual).max()),
            "Outside20MeV": int(np.count_nonzero(abs(residual) > 20)),
            "Outside100MeV": int(np.count_nonzero(abs(residual) > 100)),
        }
    return metrics, residuals


class Tee(io.StringIO):
    def write(self, text):
        sys.__stdout__.write(text)
        sys.__stdout__.flush()
        return super().write(text)


def execute_notebook(path):
    notebook = json.loads(path.read_text())
    namespace = {"__name__": "__main__"}
    count = 0
    for index, cell in enumerate(notebook["cells"]):
        if cell["cell_type"] != "code":
            continue
        source = cell["source"]
        source = source if isinstance(source, str) else "".join(source)
        ast.parse(source)
        count += 1
        cell["execution_count"] = count
        cell["outputs"] = []
        text_output = Tee()

        def show(*args, **kwargs):
            text = text_output.getvalue()
            if text:
                cell["outputs"].append({"output_type": "stream", "name": "stdout", "text": text.splitlines(True)})
                text_output.seek(0)
                text_output.truncate(0)
            for number in plt.get_fignums():
                figure = plt.figure(number)
                buffer = io.BytesIO()
                figure.savefig(buffer, format="png", dpi=120, bbox_inches="tight")
                cell["outputs"].append({"output_type": "display_data", "metadata": {},
                    "data": {"image/png": base64.b64encode(buffer.getvalue()).decode(),
                             "text/plain": [repr(figure)]}})
                plt.close(figure)

        plt.show = show
        print(f"EXECUTE {path.name} cell={index}", flush=True)
        with contextlib.redirect_stdout(text_output):
            exec(compile(source, f"{path.name}:cell{index}", "exec"), namespace)
        if text_output.getvalue():
            cell["outputs"].append({"output_type": "stream", "name": "stdout",
                                    "text": text_output.getvalue().splitlines(True)})
        path.write_text(json.dumps(notebook, ensure_ascii=False, indent=1) + "\n")
    return namespace


started = time.monotonic()
before = torch.load(RUN_DIR / "EnergyLossMLP_before.pt", map_location="cpu", weights_only=False)
datasets = load_data(before)
before_metrics, before_residuals = evaluate(before, datasets)
(RUN_DIR / "metrics_before.json").write_text(json.dumps(before_metrics, indent=2) + "\n")
training = execute_notebook(ENERGY_DIR / "EnergyLoss.ipynb")
evaluation = execute_notebook(ENERGY_DIR / "EnergyLossEvaluation.ipynb")
after = torch.load(ENERGY_DIR / "EnergyLoss_output/EnergyLossMLP.pt", map_location="cpu", weights_only=False)
after_metrics, after_residuals = evaluate(after, datasets)
assert len(training["train_loss_history"]) == 2000
assert np.isclose(after_metrics["internal_test"]["RMSE_MeV"], training["metrics"]["RMSE_MeV"])
assert np.isclose(after_metrics["external_test"]["RMSE_MeV"], evaluation["metrics"]["RMSE_MeV"])
constant = np.ptp(datasets["training"][0], axis=0) == 0
assert np.all(after["x_std"][constant] == 1)
assert np.all(after["x_mean"][constant] == datasets["training"][0][0, constant])
assert np.all(np.isfinite(after_residuals["external_test"]))
report = {"before": before_metrics, "after": after_metrics,
          "epochs": 2000, "cpu_threads": 1,
          "constant_features": [n for n, mask in zip(after["feature_names"], constant) if mask],
          "elapsed_seconds": time.monotonic() - started}
(RUN_DIR / "comparison.json").write_text(json.dumps(report, indent=2) + "\n")
np.savez(RUN_DIR / "predictions_and_losses.npz",
         **{f"before_{n}": r for n, r in before_residuals.items()},
         **{f"after_{n}": r for n, r in after_residuals.items()},
         train_loss=training["train_loss_history"], test_loss=training["test_loss_history"])
fig, axes = plt.subplots(1, 2, figsize=(12, 4.5))
for label, residual in [("Before", before_residuals["external_test"]), ("After", after_residuals["external_test"])]:
    rmse = np.sqrt(np.mean(residual**2))
    axes[0].hist(residual, bins=np.arange(-10, 10.5, 0.5), histtype="step", label=f"{label}: RMSE={rmse:.3f} MeV")
    absolute = np.sort(abs(residual))
    axes[1].step(absolute, (len(absolute) - np.arange(len(absolute))) / len(absolute), where="post", label=label)
axes[0].set(xlabel="Prediction - truth (MeV)", ylabel="Counts / 0.5 MeV", xlim=(-10, 10), title="External test: residual center")
axes[1].set(xlabel="Absolute error (MeV)", ylabel="Fraction with error at least this large", yscale="log", title="External test: full error tail")
for axis in axes:
    axis.legend()
    axis.grid(alpha=0.3)
fig.tight_layout()
fig.savefig(RUN_DIR / "comparison.png", dpi=160)
plt.close(fig)
print("COMPARISON", json.dumps(report, ensure_ascii=False), flush=True)
