#ifndef RC_SCOPED_OBSERVATION_H_
#define RC_SCOPED_OBSERVATION_H_

namespace rc {

template <typename Observer, typename Source>
class ScopedObservation {
    static_assert(requires(Observer* observer, Source& source) {
      source.AddObserver(observer);
      source.RemoveObserver(observer);
    });

  public:
    explicit ScopedObservation(Observer* observer) : observer_(observer) {
    }
    ScopedObservation(const ScopedObservation&) = delete;
    void operator=(const ScopedObservation&) = delete;

    ~ScopedObservation() {
      Reset();
    }

    void Observe(Source* source) {
      source_ = source;
      if (source_) {
        source_->AddObserver(observer_);
      }
    }

    void Reset() {
      if (source_) {
        source_->RemoveObserver(observer_);
        source_ = nullptr;
      }
    }

  private:
    Observer* observer_;
    Source* source_{nullptr};
};

} // namespace rc

#endif // !RC_SCOPED_OBSERVATION_H_
