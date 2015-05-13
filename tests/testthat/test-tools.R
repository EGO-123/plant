context("Tools")

test_that("fixed_environment", {
  env <- fixed_environment(0.5)
  expect_that(env, is_a("Environment"))
  expect_that(env$light_environment$xy,
              equals(cbind(c(0, 75, 150), 0.5)))
  expect_that(env$canopy_openness(40), equals(0.5))
})

test_that("lcp_whole_plant", {
  ## R implementation:
  lcp_whole_plant_R <- function(plant, ...) {
    target <- function(canopy_openness) {
      env <- fixed_environment(canopy_openness)
      plant$compute_vars_phys(env)
      plant$internals[["net_mass_production_dt"]]
    }

    f1 <- target(1)
    if (f1 < 0.0) {
      NA_real_
    } else {
      uniroot(target, c(0, 1), f.upper=f1, ...)$root
    }
  }

  p <- FFW16_PlantPlus(FFW16_Strategy())
  expect_that(lcp_whole_plant(p),
              equals(lcp_whole_plant_R(p), tolerance=1e-5))
})