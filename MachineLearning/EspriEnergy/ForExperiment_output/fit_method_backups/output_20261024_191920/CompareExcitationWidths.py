#!/usr/bin/env python3
# 功能：复用 ForExperiment_output 的同事件激发能，比较原实验重建与 MLP 的基态、第一激发态高斯 σ。
# 方法：iminuit 非分箱扩展似然拟合两个独立高斯峰和指数背景；峰位自由，截断概率归一化；
#       按左右臂及合并样本拟合，改变公共拟合窗口检验稳定性，保存 CSV、JSON、PDF/PNG 与说明。
# 注意事项：能量单位为 MeV；原重建并非真值；HESSE 误差不含模型系统误差，两个方法使用相关事件；
#           不平移或校正数据，不重新推理或训练，不改既有产物；每次新建秒级时间戳输出目录。
from datetime import datetime
from pathlib import Path
import csv
import hashlib
import json

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from iminuit import Minuit
from scipy.special import ndtr, xlogy
from scipy.stats import chi2


PROJECT = Path(__file__).resolve().parent
INPUT = PROJECT / "ForExperiment_output" / "ExcitationEnergyComparison.npz"
FIT_RANGE = (-2.0, 5.8)
BIN_WIDTH = 0.1
PARAMETERS = ("n0", "mu0", "sigma0", "n1", "mu1", "sigma1", "nb", "slope")


def components(x, parameters, bounds, cumulative=False):
    """返回以拟合窗口内产额归一化的基态、第一激发态及背景。"""
    n0, mu0, sigma0, n1, mu1, sigma1, nb, slope = parameters
    low, high = bounds
    values = []
    for count, mean, sigma in ((n0, mu0, sigma0), (n1, mu1, sigma1)):
        norm = ndtr((high - mean) / sigma) - ndtr((low - mean) / sigma)
        if cumulative:
            density = (ndtr((x - mean) / sigma) - ndtr((low - mean) / sigma)) / norm
        else:
            density = np.exp(-0.5 * ((x - mean) / sigma) ** 2) / (np.sqrt(2 * np.pi) * sigma * norm)
        values.append(count * density)
    norm = np.expm1(slope * (high - low))
    if cumulative:
        background = np.expm1(slope * (x - low)) / norm
    else:
        background = slope * np.exp(slope * (x - low)) / norm
    values.append(nb * background)
    return np.asarray(values)


def fit_spectrum(values, method, bounds, initial=None):
    low, high = bounds
    data = np.asarray(values[(values >= low) & (values <= high)], dtype=np.float64)

    def nll(n0, mu0, sigma0, n1, mu1, sigma1, nb, slope):
        parameters = (n0, mu0, sigma0, n1, mu1, sigma1, nb, slope)
        density = components(data, parameters, bounds).sum(axis=0)
        return n0 + n1 + nb - np.log(density).sum()

    shift = 0.0 if method == "Original" else 0.7
    if initial is None:
        initial = (0.13 * len(data), shift, 0.6, 0.66 * len(data), 3.3 + shift, 0.65, 0.21 * len(data), 0.4)
    fit = Minuit(nll, *initial, name=PARAMETERS)
    fit.errordef = Minuit.LIKELIHOOD
    for name in ("n0", "n1", "nb"):
        fit.limits[name] = (0.0, 2.0 * len(data))
    fit.limits["mu0"] = (-1.0, 1.8)
    fit.limits["mu1"] = (2.6, 4.8)
    for name in ("sigma0", "sigma1"):
        fit.limits[name] = (0.15, 1.4)
    fit.limits["slope"] = (0.001, 3.0)
    fit.migrad()
    fit.hesse()
    edges = np.linspace(low, high, round((high - low) / BIN_WIDTH) + 1)
    observed = np.histogram(data, edges)[0]
    expected = np.diff(components(edges, tuple(fit.values), bounds, cumulative=True).sum(axis=0))
    deviance = float(2.0 * np.sum(expected - observed + xlogy(observed, observed / expected)))
    ndf = len(observed) - fit.nfit
    quality = {
        "valid_minimum": bool(fit.valid),
        "accurate_covariance": bool(fit.accurate),
        "parameters_at_limit": bool(fit.fmin.has_parameters_at_limit),
        "edm": float(fit.fmin.edm),
        "poisson_deviance": deviance,
        "ndf": ndf,
        "deviance_per_ndf": deviance / ndf,
        "approximate_gof_pvalue": float(chi2.sf(deviance, ndf)),
    }
    result = {
        "method": method, "range_MeV": list(bounds), "fit_events": len(data),
        "parameters": {name: float(fit.values[name]) for name in PARAMETERS},
        "errors": {name: float(fit.errors[name]) for name in PARAMETERS},
        "covariance_parameter_order": list(PARAMETERS),
        "covariance": np.asarray(fit.covariance).tolist(),
        "quality": quality,
    }
    if not fit.valid or not fit.accurate or fit.fmin.has_parameters_at_limit:
        raise RuntimeError(f"Fit requires review: {method}, {bounds}, {quality}")
    return result


def write_plot(output, arrays, results):
    fig, axes = plt.subplots(2, 2, figsize=(13.2, 7.3), sharex="col",
                             gridspec_kw={"height_ratios": [4, 1]}, layout="constrained")
    edges = np.linspace(*FIT_RANGE, round((FIT_RANGE[1] - FIT_RANGE[0]) / BIN_WIDTH) + 1)
    centers = (edges[:-1] + edges[1:]) / 2.0
    x = np.linspace(*FIT_RANGE, 1200)
    for column, method in enumerate(("Original", "MLP")):
        result = results[method]
        parameters = tuple(result["parameters"][name] for name in PARAMETERS)
        observed = np.histogram(arrays[method], edges)[0]
        ax, pull_ax = axes[:, column]
        ax.errorbar(centers, observed, yerr=np.sqrt(observed), fmt="o", markersize=2.4,
                    color="#000000", elinewidth=0.6, capsize=0)
        curves = components(x, parameters, FIT_RANGE) * BIN_WIDTH
        ax.plot(x, curves.sum(axis=0), color="blue", linewidth=2, alpha=0.8)
        ax.plot(x, curves[0], color="red", linewidth=1, alpha=0.8)
        ax.plot(x, curves[1], color="red", linestyle="--", linewidth=1, alpha=0.8)
        ax.plot(x, curves[2], color="black", linestyle="--", linewidth=1, alpha=0.8)
        ax.set_title("Original experimental reconstruction" if method == "Original" else "MLP reconstruction")
        ax.set_ylabel(f"Counts / {BIN_WIDTH:g} MeV")
        ax.set_ylim(0, 1.27 * max(observed))
        ax.set_box_aspect(3 / 4)
        p, e = result["parameters"], result["errors"]
        annotation = (
            f"Ground state\n"
            rf"$\mu={p['mu0']:.3f}$, $\sigma={p['sigma0']:.3f}\pm{e['sigma0']:.3f}$ MeV" "\n"
            f"First excited state\n"
            rf"$\mu={p['mu1']:.3f}$, $\sigma={p['sigma1']:.3f}\pm{e['sigma1']:.3f}$ MeV" "\n"
            f"Deviance / ndf = {result['quality']['deviance_per_ndf']:.2f}"
        )
        ax.text(0.025, 0.965, annotation, transform=ax.transAxes, va="top", fontsize=9)
        ax.text(0.98, 0.97, f"({chr(97 + column)})", transform=ax.transAxes, ha="right", va="top")
        expected = np.diff(components(edges, parameters, FIT_RANGE, cumulative=True).sum(axis=0))
        pull = (observed - expected) / np.sqrt(expected)
        pull_ax.axhline(0, color="blue", linewidth=0.8)
        pull_ax.axhline(3, color="black", linestyle="--", linewidth=0.6, alpha=0.4)
        pull_ax.axhline(-3, color="black", linestyle="--", linewidth=0.6, alpha=0.4)
        pull_ax.plot(centers, pull, "o", markersize=2.4, color="black")
        pull_ax.set(xlabel="Excitation energy (MeV)", ylabel="Pull", xlim=FIT_RANGE,
                    ylim=(-max(4, np.ceil(abs(pull).max())), max(4, np.ceil(abs(pull).max()))))
        for panel in (ax, pull_ax):
            panel.tick_params(direction="in", top=True, right=True)
    fig.suptitle(r"Left + Right, $55^\circ\leq\theta_p\leq70^\circ$, same 39,860 events", fontsize=13)
    fig.supxlabel("Blue: total; red solid: ground state; red dashed: first excited state; black dashed: background.", fontsize=10)
    fig.canvas.draw()
    for ax in axes[1]:
        ticks = [tick for tick in ax.xaxis.get_major_ticks() if FIT_RANGE[0] <= tick.get_loc() <= FIT_RANGE[1]]
        if len(ticks) > 1:
            ticks[0].label1.set_visible(False)
            ticks[-1].label1.set_visible(False)
    for suffix in ("pdf", "png"):
        fig.savefig(output / f"ExcitationGaussianFits.{suffix}", dpi=180, bbox_inches="tight")
    plt.close(fig)


def main():
    with np.load(INPUT) as saved:
        pair = saved["PairOK"]
        if not np.array_equal(pair, saved["OriginalOK"] & saved["NNOK"]):
            raise ValueError("PairOK 与两列有效性不一致")
        if len(np.unique(saved["SourceEntry"][pair])) != int(pair.sum()):
            raise ValueError("同事件样本包含重复 SourceEntry")
        arrays = {"Original": saved["ExOriginal_MeV"][pair], "MLP": saved["ExNN_MeV"][pair]}
        sides = saved["espriSide"][pair]
        for method, source in (("Original", "Original"), ("MLP", "NN")):
            if not np.isfinite(arrays[method]).all():
                raise ValueError("激发能包含非有限值")
            if not np.array_equal(np.histogram(arrays[method], saved["edges_MeV"])[0], saved[source]):
                raise ValueError("事件数组与既有直方图不一致")
    output = PROJECT / "ForExperiment_output" / datetime.now().strftime("output_%Y%m%d_%H%M%S")
    output.mkdir(exist_ok=False)
    fits, rows = {}, []
    for sample, selected in (("Combined", np.ones(len(sides), dtype=bool)), ("Left", sides == 1), ("Right", sides == 0)):
        fits[sample] = {}
        for method, values in arrays.items():
            fit = fit_spectrum(values[selected], method, FIT_RANGE)
            fits[sample][method] = fit
            for state, suffix in (("gs", "0"), ("first", "1")):
                rows.append({"sample": sample, "method": method, "state": state,
                             "mu_MeV": fit["parameters"][f"mu{suffix}"],
                             "mu_err_MeV": fit["errors"][f"mu{suffix}"],
                             "sigma_MeV": fit["parameters"][f"sigma{suffix}"],
                             "sigma_err_MeV": fit["errors"][f"sigma{suffix}"],
                             "deviance_per_ndf": fit["quality"]["deviance_per_ndf"]})
    sensitivity = []
    for bounds in ((-2.0, 5.5), (-2.0, 6.1), (-2.3, 5.8), (-1.7, 5.8)):
        for method, values in arrays.items():
            initial = tuple(fits["Combined"][method]["parameters"][name] for name in PARAMETERS)
            sensitivity.append(fit_spectrum(values, method, bounds, initial))
    summary = {
        "input": str(INPUT), "input_sha256": hashlib.sha256(INPUT.read_bytes()).hexdigest(),
        "common_events": len(sides), "fit_model": "Two independent Gaussians + exponential background",
        "fit_method": "Unbinned extended maximum likelihood; free peak means and widths; iminuit MIGRAD/HESSE",
        "errors": "Per-fit HESSE statistical errors; same-event methods are correlated; no cross-method error inferred",
        "scope": "Original experimental reconstruction is a reference, not truth; widths include angle/arm mixing",
        "fits": fits, "range_sensitivity": sensitivity,
    }
    (output / "FitSummary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2, allow_nan=False), encoding="utf-8")
    with (output / "GaussianWidths.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    write_plot(output, arrays, fits["Combined"])
    report = ["# 基态与第一激发态高斯宽度对比", "",
              f"同事件 {len(sides):,} 个；沿用 55–70°、BTZ 5σ、Phi12 cut。",
              "原实验重建和 MLP 均对同一公共区间 −2.0–5.8 MeV 作两个高斯峰加指数背景的非分箱扩展似然拟合。",
              "峰位和宽度独立自由变化；没有平移谱或修正 MLP 能量。误差仅为单次拟合的 HESSE 统计误差。", "",
              "| 态 | 原实验 σ (MeV) | MLP σ (MeV) | MLP / 原实验 | 峰位偏移 (MeV) |", "|---|---:|---:|---:|---:|"]
    for state, suffix in (("基态", "0"), ("第一激发态", "1")):
        old, new = fits["Combined"]["Original"], fits["Combined"]["MLP"]
        a, b = old["parameters"][f"sigma{suffix}"], new["parameters"][f"sigma{suffix}"]
        da, db = old["errors"][f"sigma{suffix}"], new["errors"][f"sigma{suffix}"]
        shift = new["parameters"][f"mu{suffix}"] - old["parameters"][f"mu{suffix}"]
        report.append(f"| {state} | {a:.4f} ± {da:.4f} | {b:.4f} ± {db:.4f} | {b/a:.4f} | {shift:+.4f} |")
    report += ["", "公共拟合区间变化检验（非完整系统误差）：", "", "| 方法 | 基态 σ 范围 (MeV) | 第一激发态 σ 范围 (MeV) |", "|---|---:|---:|"]
    for method in arrays:
        selected_fits = [fits["Combined"][method]] + [item for item in sensitivity if item["method"] == method]
        ranges = []
        for name in ("sigma0", "sigma1"):
            values = [item["parameters"][name] for item in selected_fits]
            ranges.append(f"{min(values):.4f}–{max(values):.4f}")
        report.append(f"| {method} | {ranges[0]} | {ranges[1]} |")
    report += ["", "所有拟合均检查最小值、协方差及参数边界；逐臂结果、拟合优度和协方差见 JSON/CSV。",
               "原实验能量不是已知真值；本结果是选样后的重建峰宽，包含角度和两臂混合，不能等同于独立的探测器本征分辨率。",
               "两套结果来自同一批事件，未用独立误差假设计算宽度差或比值的显著性。",
               "窗口变化中第一激发态尾部可能混入更高能结构，范围检验不代表完整背景模型系统误差。", ""]
    (output / "Comparison.md").write_text("\n".join(report), encoding="utf-8")
    print("\n".join(report))
    print("Output:", output)


if __name__ == "__main__":
    main()
